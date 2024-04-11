#ifndef UGEM_H__
#define UGEM_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define UGEM_ENV_KEY "UGEM_KEY"
#define UGEM_ENV_CERT "URGME_CERT"
#define UGEM_DEFAULT_KEY "./server.key"
#define UGEM_DEFAULT_CERT "./server.cert"

#define UGEM_DEFAULT_PORT 1965;
#define UGEM_DEFAULT_SA_FAMILY AF_INET

extern FILE *ugemin;
extern FILE *ugemout;
extern FILE *ugemerr;

struct ugem {
  int server_listening;
};

struct ugem_config {
  char **argv;
  int argc;

  int verbose;

  const char *key_path;
  const char *cert_path;

  int port;
  sa_family_t sa_family;
};

extern struct ugem ugem;
extern struct ugem_config ugemcfg;

void ugem_init(struct ugem_config cfg);

struct ugem_config ugem_cfg_init(void);

int ugem_main(struct ugem_config cfg);

#endif
