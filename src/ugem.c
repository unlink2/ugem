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
#include <time.h>

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
  case UGEM_FAIL_NOT_FOUND:
    return "NOT FOUND";
  case UGEM_FAIL_GONE:
    return "GONE";
  case UGEM_FAIL_PROXY_REFUSED:
    return "PROXY REFUSED";
  case UGEM_FAIL_BAD_REQUEST:
    return "BAD REQUEST";
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

// FIXME: this is likely no good for windows as of now! (e.g. need to check
// <dirve letter> and
//        backslash variations for parent directory
int ugem_is_path_valid(const char *path, unsigned long n) {
  return n == 0 || (path[0] != '/' && !strstr(path, "../") &&
                    (n < 3 || strncmp(path + n - 3, "/..", 3) != 0));
}

int ugem_path_join(char *dst, const char *p1, const char *p2, char sep,
                   unsigned long n) {
  return snprintf(dst, n, "%s%c%s", p1, sep, p2);
}

// handles routing rules
// returns 0 if no rule has been matched
// in which case the default rule (serve from file system) should apply
int ugem_handle_rules(void *connection, struct ugem_uri *uri,
                      struct ugem_host_config *hostcfg) {
  return 0;
}

enum ugem_status ugem_handle(void *connection, struct ugem_request request,
                             struct ugem_host_config *hostcfg) {
  enum ugem_status status = UGEM_TMP_FAIL_UNSPECIFIED;
  struct ugem_uri uri =
      ugem_uri_parse(request.payload, ugemcfg.port, request.payload_len);

  // validate we are getting a request for the correct host!
  if (hostcfg->host != NULL && strcmp(uri.host, hostcfg->host) != 0) {
    ugem_log(ugemerr, "%d: incorrect host in request\n", request.trace);
    status = UGEM_FAIL_BAD_REQUEST;
    goto fail;
  }

  if (!ugem_is_path_valid(uri.path, strlen(uri.path))) {
    ugem_log(ugemerr, "%d: bad request path\n", request.trace);
    status = UGEM_FAIL_BAD_REQUEST;
    goto fail;
  }

fail:
  if (UGEM_SHOULD_LOG(UGEM_INFO)) {
    ugem_log(ugemerr, "%d: request '", request.trace);
    ugem_print_payload(ugemerr, request.payload, request.payload_len);
    fprintf(ugemerr, "' from %s:%d returned with status %s\n", request.src_addr,
            request.src_port, ugem_status_tostr(status));
  }

  ugem_uri_free(&uri);

  return status;
}

int ugem_gettrace(void) { return ugem.next_trace++; }

int ugem_main(struct ugem_config cfg) {
  ugem_init(cfg);

  if (signal(SIGINT, ugem_sig_handler) == SIG_ERR) {
    ugem_log(ugemerr, "Unable to catch SIGINT\n");
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
    ugem_log(ugemerr, "Listening on port %d for host: %s\n", ugemcfg.port,
            ugemcfg.hostcfg.host);
  }

  char buf[UGEM_NET_BUF_MAX];
  unsigned long buf_len = UGEM_NET_BUF_MAX;

  while (ugem.server_listening) {
    struct sockaddr_in addr;
    unsigned int addr_len = 0;
    int client_fd = accept(ugem.server_fd, (struct sockaddr *)&addr, &addr_len);
    if (client_fd <= 0) {
      continue;
    }
    const char *saddr = inet_ntoa(addr.sin_addr);

    int trace = ugem_gettrace();

    if (UGEM_SHOULD_LOG(UGEM_INFO)) {
      ugem_log(ugemerr, "%d: %s connected\n", trace, saddr);
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
      ugem_log(ugemerr, "Read returned %ld\n", read);
      goto disconnect;
    }

    const char *test =
        "20 text/gemini\r\n# Hello world\r\nThis is a test message!\r\n";

    struct ugem_request request = {.payload = buf,
                                   .payload_len = read,
                                   .src_addr = saddr,
                                   .src_port = addr.sin_port,
                                   .trace = trace};
    ugem_handle(connection, request, &ugemcfg.hostcfg);

    if (ugem_net_secure_write(connection, test, strlen(test)) <= 0) {
      ugem_log(ugemerr, "%s:%d : Write failed!\n", request.src_addr,
              request.src_port);
    }

  disconnect:

    ugem_net_secure_disconnect(connection, client_fd);
    if (UGEM_SHOULD_LOG(UGEM_INFO)) {
      ugem_log(ugemerr, "%d: %s disconnected\n", trace, saddr);
    }
  }

  ugem_net_secure_ctx_free(server_secure_ctx);
  ugem_net_socket_close(ugem.server_fd);

  return 0;
}
