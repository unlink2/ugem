#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ugem.h"
#include <unistd.h>

#define UGEM_NAME "ugem"
#define UGEM_VER "0.0.1"

// args without value
#define UGEM_OPTS "hvV"

// args with value e.g. o:
#define UGEM_OPTS_ARG ""

#define UGEM_HELP(a, desc) printf("\t-%s\t%s\n", (a), desc);

void ugem_help(void) {
  printf("%s\n", UGEM_NAME);
  printf("Usage %s [%s]\n\n", UGEM_NAME, UGEM_OPTS);
  UGEM_HELP("h", "display this help and exit");
  UGEM_HELP("V", "display version info and exit");
  UGEM_HELP("v", "verbose output");
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
    case 'v':
      cfg->verbose = 0;
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

  int res = ugem_main(cfg);

  return res;
}
