#include "ugem.h"
#include "net.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

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
    fprintf(ugemerr, "Listening on port %d\n", ugemcfg.port);
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

    const char *test = "20 text/gemini\r\n# Hello world\r\nThis is a test message!\r\n";

    if (ugem_net_secure_write(connection, test, strlen(test)) <= 0) {
      fprintf(ugemerr, "Write failed!\n");
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
