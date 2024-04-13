#include "uri.h"
#include "ugem.h"
#include <ctype.h>

unsigned long ugem_tok_until(const char *src, char until,
                             enum ugem_tok_flags flags, long n) {
  unsigned long read = 0;
  if (n <= 0) {
    return 0;
  }

  for (read = 0; read < n && src[read] && src[read] != until; read++) {
  }

  if (src[read] == '\0' && (flags & UGEM_TOK_OR_END) == 0) {
    return 0;
  }

  return read;
}

char *ugem_strndup(const char *src, unsigned long n) {
  char *dst = ugem_malloc(n + 1);
  strncpy(dst, src, n);
  dst[n] = '\0';
  return dst;
}

struct ugem_uri ugem_uri_parse(const char *uri_str, int default_port, long n) {
  long readat = 0;
  struct ugem_uri uri = {.err = 1};
  if (n == 0) {
    fprintf(ugemerr, "Attempting to parse an uri of length 0\n");
    return uri;
  }

  // scheme:
  {
    unsigned int len = ugem_tok_until(uri_str+readat, ':', 0, n-readat);
    if (len == 0) {
      fprintf(ugemerr, "Unabel to parse uri scheme of '%s'\n", uri_str);
      return uri;
    }

    uri.scheme = ugem_strndup(uri_str + readat, len);
    readat += len + 1;
  }
  

  // consume //

  // host

  // optional :

  // if : get port

  // optional / at the end

  // if / get path

  // optional #

  // if # get fragment

  // optional ?

  // if ? get key=value
  // repeat until end is not &

  uri.err = 0;
  return uri;
}

int ugem_uri_reserved(int c) {
  switch (c) {
  case ';':
  case '/':
  case '?':
  case '@':
  case '&':
  case '=':
    return 1;
  default:
    break;
  }

  return 0;
}

unsigned long ugem_uri_unescape(char *dst, const char *src, long n) {
  if (n == 0) {
    return 0;
  }
  const char uri_escape_char = '%';

  unsigned long writeat = 0;
  unsigned long readat = 0;

  for (readat = 0; readat < n && src[readat]; readat++) {
    char c = src[readat];
    if (c == uri_escape_char) {
      if (readat + 3 >= n) {
        goto fail;
      }

      char hexstr[3] = {src[readat + 1], src[readat + 2], '\0'};

      if (!isxdigit(hexstr[0]) || !isxdigit(hexstr[1])) {
        goto fail;
      }

      c = (char)strtol(hexstr, NULL, 16);

      readat += 2; // + 2 to consume the 2 hex chars
    }
    dst[writeat++] = c;
  }

  dst[writeat] = '\0';

  return 0;

fail:
  dst[0] = '\0';
  return readat;
}

const char *ugem_uri_escape(const char *src, unsigned long n) {}

void ugem_uri_free(struct ugem_uri *uri) {}
