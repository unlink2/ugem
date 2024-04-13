#ifndef URI_H_
#define URI_H_

struct ugem_query {
  const char *key;
  const char *value;
};

struct ugem_uri {
  const char *scheme;
  const char *authority;

  int port;

  const char *path;

  struct ugem_query *query;
  unsigned long query_len;

  const char *fragment;
};

struct ugem_uri ugem_uri_parse(const char *uri_str, unsigned long n);

// unescapes a string if n size
// On success 0 is returned, on failure 
// the last read index of src is returned.
// src and dst must be of size n+1.
// dst is automatically terminated with \0
unsigned long ugem_uri_unescape(char *dst, const char *src, unsigned long n);

void ugem_uri_free(struct ugem_uri *uri);

#endif 
