#include "ugem.h"
#include "uri.h"
#include "net.h"
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *ugemin = NULL;
FILE *ugemout = NULL;
FILE *ugemerr = NULL;
struct ugem ugem;
struct ugem_config ugemcfg;

void ugem_init(struct ugem_config cfg) {
  if (!ugemin) {
    ugemin = stdin;
  }
  if (!ugemout) {
    ugemout = stdout;
  }
  if (!ugemerr) {
    ugemerr = stderr;
  }

  ugemcfg = cfg;
  memset(&ugem, 0, sizeof(ugem));

  ugem.server_listening = 1;
}

struct ugem_config ugem_cfg_init(void) {
  struct ugem_config cfg;
  memset(&cfg, 0, sizeof(cfg));

  cfg.key_path = getenv(UGEM_ENV_KEY);
  if (!cfg.key_path) {
    cfg.key_path = UGEM_DEFAULT_KEY;
  }

  cfg.cert_path = getenv(UGEM_ENV_CERT);
  if (!cfg.cert_path) {
    cfg.cert_path = UGEM_DEFAULT_CERT;
  }

  // TODO: env for port
  cfg.port = UGEM_DEFAULT_PORT;

  // TODO: env for sa family
  cfg.sa_family = UGEM_DEFAULT_SA_FAMILY;

  cfg.hostcfg.root_path = getenv(UGEM_ENV_ROOT_DIR);
  if (!cfg.hostcfg.root_path) {
    cfg.hostcfg.root_path = UGEM_DEFAULT_ROOT_DIR;
  }

  return cfg;
}

void ugem_sig_handler(int signo) {
  if (signo == SIGINT) {
    ugem.server_listening = 0;

    // we connect to the server socket
    ugem_net_socket_close(ugem.server_fd);
    exit(0);
  }
}

const char *ugem_status_tostr(enum ugem_status status) {
  switch (status) {
  case UGEM_INPUT:
    return "INPUT";
  case UGEM_INPUT_SENSITIVE:
    return "SENSITIVE INPUT";
  case UGEM_SUCCESS:
    return "SUCCESS";
  case UGEM_REDIRECT_TEMP:
    return "TEMPORARY REDIRECT";
  case UGEM_REDIRECT_PERM:
    return "PERMANENT REDIRECT";
  case UGEM_TMP_FAIL_UNSPECIFIED:
    return "TEMPORARY FAILURE";
  case UGEM_TMP_FAIL_SERVER_UNAVAIL:
    return "SERVER UNAVAILABLE";
  case UGEM_TMP_FAIL_CGI_ERR:
    return "CGI ERROR";
  case UGEM_TMP_FAIL_SLOW_DOWN:
    return "SLOW DOWN";
  case UGEM_CERT_REQUIRED:
    return "CERTIFICATE REQUIRED";
  case UGEM_CERT_NOT_AUTH:
    return "UNAUTHORIZED";
  case UGEM_CERT_INVALID:
    return "CERTIFICATE INVALID";
  default:
    return "unknown";
  }
  return "unknown";
}

void ugem_print_payload(FILE *f, const char *buf, long read) {
  for (int i = 0; i < read; i++) {
    if (isprint(buf[i])) {
      fputc(buf[i], ugemerr);
    } else {
      fprintf(ugemerr, "\\x%02x", (char)buf[i]);
    }
  }
}

// checks if path can be used
// or if it at any point contains .. that
// maye lead to escaping the root directory
int ugem_is_path_valid(const char *path, unsigned long n) {
  return strstr(path, "../") == NULL; 
}

enum ugem_status ugem_handle(void *connection, struct ugem_request request,
                             struct ugem_host_config *hostcfg) {
  enum ugem_status status = UGEM_TMP_FAIL_UNSPECIFIED;
  struct ugem_uri uri =
      ugem_uri_parse(request.payload, ugemcfg.port, request.payload_len);
  if (hostcfg->host != NULL && strcmp(uri.host, hostcfg->host) != 0) {
    status = UGEM_FAIL_BAD_REQUEST;
    goto fail;
  }

  if (!ugem_is_path_valid(uri.path, strlen(uri.path))) {
    status = UGEM_FAIL_BAD_REQUEST;
    goto fail;
  }

fail:
  if (UGEM_SHOULD_LOG(UGEM_INFO)) {
    fprintf(ugemerr, "request '");
    ugem_print_payload(ugemerr, request.payload, request.payload_len);
    fprintf(ugemerr, "' from %s:%d returned with status %s\n", request.src_addr,
            request.src_port, ugem_status_tostr(status));
  }
  return status;
}

int ugem_main(struct ugem_config cfg) {
  ugem_init(cfg);

  if (signal(SIGINT, ugem_sig_handler) == SIG_ERR) {
    fprintf(ugemerr, "Unable to catch SIGINT\n");
    return -1;
  }

  ugem.server_fd = ugem_net_server_socket_init(cfg.port, cfg.sa_family);

  if (ugem.server_fd < 0) {
    return -1;
  }

  void *server_secure_ctx = ugem_net_secure_ctx_init();
  if (!server_secure_ctx) {
    return -1;
  }

  if (UGEM_SHOULD_LOG(UGEM_INFO)) {
    fprintf(ugemerr, "Listening on port %d for host: %s\n", ugemcfg.port,
            ugemcfg.hostcfg.host);
  }

  char buf[4096];
  unsigned long buf_len = 4096;

  while (ugem.server_listening) {
    struct sockaddr_in addr;
    unsigned int addr_len = 0;
    int client_fd = accept(ugem.server_fd, (struct sockaddr *)&addr, &addr_len);
    if (client_fd <= 0) {
      continue;
    }
    const char *saddr = inet_ntoa(addr.sin_addr);

    if (UGEM_SHOULD_LOG(UGEM_INFO)) {
      fprintf(ugemerr, "%s connected\n", saddr);
    }

    void *connection = ugem_net_secure_handshake(server_secure_ctx, client_fd);

    if (!connection) {
      goto disconnect;
    }

    long read = 0;
    read = ugem_net_secure_read(connection, buf, buf_len);
    if (read == 0) {
      goto disconnect;
    } else if (read < 0) {
      fprintf(ugemerr, "Read returned %ld\n", read);
      goto disconnect;
    }

    const char *test =
        "20 text/gemini\r\n# Hello world\r\nThis is a test message!\r\n";

    struct ugem_request request = {.payload = buf,
                                   .payload_len = read,
                                   .src_addr = saddr,
                                   .src_port = addr.sin_port};
    ugem_handle(connection, request, &ugemcfg.hostcfg);

    if (ugem_net_secure_write(connection, test, strlen(test)) <= 0) {
      fprintf(ugemerr, "%s:%d : Write failed!\n", request.src_addr,
              request.src_port);
    }

  disconnect:

    ugem_net_secure_disconnect(connection, client_fd);
    if (UGEM_SHOULD_LOG(UGEM_INFO)) {
      fprintf(ugemerr, "%s disconnected\n", saddr);
    }
  }

  ugem_net_secure_ctx_free(server_secure_ctx);
  ugem_net_socket_close(ugem.server_fd);

  return 0;
}
