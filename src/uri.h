#ifndef URI_H_
#define URI_H_

struct ugem_query {
  const char *key;
  const char *value;
};

struct ugem_uri {
  const char *scheme;
  const char *domain;

  int port;

  const char *path;

  struct ugem_query *query;
  unsigned long query_len;
};

#endif 
