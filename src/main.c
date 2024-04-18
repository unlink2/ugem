#include <stdio.h>
#include <stdlib.h>
#include "ugem.h"
#include <unistd.h>

#define UGEM_NAME "ugem"
#define UGEM_VER "0.0.1"

// args without value
#define UGEM_OPTS "hvV46a"

// args with value e.g. o:
#define UGEM_OPTS_ARG "H:b:"

#define UGEM_HELP(a, desc) printf("\t-%s\t%s\n", (a), desc);

void ugem_help(void) {
  printf("%s\n", UGEM_NAME);
  printf("Usage %s [%s] [root directory]\n\n", UGEM_NAME, UGEM_OPTS);
  UGEM_HELP("h", "display this help and exit");
  UGEM_HELP("V", "display version info and exit");
  UGEM_HELP("H", "Set accepted host");
  UGEM_HELP("a", "Show all files in directory index");
  UGEM_HELP("b", "Bind socket to specific address (defaults to 0.0.0.0)");
  UGEM_HELP("v", "increase log level");
  UGEM_HELP("4", "Use IPv4");
  UGEM_HELP("6", "Use IPv6");
}

void ugem_version(void) { printf("%s version %s\n", UGEM_NAME, UGEM_VER); }

void ugem_getopt(int argc, char **argv, struct ugem_config *cfg) {
  int c = 0;
  while ((c = getopt(argc, argv, UGEM_OPTS UGEM_OPTS_ARG)) != -1) {
    switch (c) {
    case 'h':
      ugem_help();
      exit(0);
      break;
    case 'V':
      ugem_version();
      exit(0);
      break;
    case 'H':
      cfg->hostcfg.host = optarg;
      break;
    case 'a':
      cfg->hostcfg.all = 1;
      break;
    case 'b':
      cfg->bind_addr = optarg;
      break;
    case 'v':
      cfg->verbose++;
      break;
    case '4':
      cfg->sa_family = AF_INET;
      break;
    case '6':
      cfg->sa_family = AF_INET6;
      break;
    case '?':
      break;
    default:
      printf("%s: invalid option '%c'\nTry '%s -h' for more information.\n",
             UGEM_NAME, c, UGEM_NAME);
      exit(-1);
      break;
    }
  }

  cfg->argc = argc - optind;
  cfg->argv = argv + optind;
}

int main(int argc, char **argv) {
  // map args to cfg here
  struct ugem_config cfg = ugem_cfg_init();

  ugem_getopt(argc, argv, &cfg);

  if (cfg.argc > 0) {
    cfg.hostcfg.root_path = cfg.argv[0];
  }

  int res = ugem_main(cfg);

  return res;
}
