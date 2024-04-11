#include "ugem.h"
#include "net.h"
#include <signal.h>
#include <unistd.h>

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
  }
}

int ugem_main(struct ugem_config cfg) {
  ugem_init(cfg);

  if (signal(SIGINT, ugem_sig_handler) == SIG_ERR) {
    fprintf(ugemerr, "Unable to catch SIGINT\n");
    return -1;
  }

  int server_socket = ugem_net_server_socket_init(cfg.port, cfg.sa_family);

  if (server_socket < 0) {
    return -1;
  }

  void *server_secure_ctx = ugem_net_secure_ctx_init();
  if (!server_secure_ctx) {
    return -1;
  }

  while (ugem.server_listening) {
  }

  ugem_net_secure_ctx_free(server_secure_ctx);
  ugem_net_socket_close(server_socket);

  return 0;
}
