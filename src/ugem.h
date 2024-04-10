#ifndef UGEM_H__
#define UGEM_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern FILE *ugemin;
extern FILE *ugemout;
extern FILE *ugemerr;

struct ugem {
  // TODO: remplace me
  int state;
};

struct ugem_config {
  char **argv;
  int argc;

  int verbose;
};

extern struct ugem ugem;
extern struct ugem_config ugemcfg;

void ugem_init(struct ugem_config cfg);

struct ugem_config ugem_cfg_init(void);

int ugem_main(struct ugem_config cfg);

#endif
