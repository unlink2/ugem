#ifndef URI_H_
#define URI_H_

struct ugem_query {
  const char *key;
  const char *value;
};

struct ugem_uri {
  int err;

  char *scheme;
  char *host;

  int port;

  char *path;

  struct ugem_query *query;
  unsigned long query_len;

  char *fragment;
};

// parse a url and return a struct containing all of its components
// this struct must be freed using ugem_uri_free
struct ugem_uri ugem_uri_parse(const char *uri_str, int default_port,
                               long n);

// unescapes a string if n size
// On success 0 is returned, on failure
// the last read index of src is returned.
// src and dst must be of size n+1.
// dst is automatically terminated with \0
unsigned long ugem_uri_unescape(char *dst, const char *src, long n);

enum ugem_tok_flags { UGEM_TOK_OR_END = 1 };

// tokenises a string until it sees 'until'
// flags:
//    UGEM_TOK_OR_END: if set it will also succeed if the string ends before
//    until is seen
// returns 0 if until could not be found
// otherwise returns the number of bytes that were consumed until 'until'
// was seen (not including 'until')
unsigned long ugem_tok_until(const char *src, char until,
                             enum ugem_tok_flags flags, long n);

void ugem_uri_free(struct ugem_uri *uri);

#endif
