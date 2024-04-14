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

  if ((src[read] == '\0' || src[read] != until) &&
      (flags & UGEM_TOK_OR_END) == 0) {
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

#define UGEM_URI_PARSE_ADV(by)                                                 \
  {                                                                            \
    n -= (by);                                                                 \
    uri_str += (by);                                                           \
  }

struct ugem_uri ugem_uri_parse(const char *uri_str, int default_port, long n) {
  const char *start_uri = uri_str;

  struct ugem_uri uri = {.err = 1};
  if (n == 0) {
    fprintf(ugemerr, "Attempting to parse an uri of length 0\n");
    goto fail;
  }

  // scheme:
  {
    unsigned int len = ugem_tok_until(uri_str, ':', 0, n);
    if (len == 0) {
      fprintf(ugemerr, "Unabel to parse uri scheme of '%s'\n", start_uri);
      goto fail;
    }

    uri.scheme = ugem_strndup(uri_str, len);

    UGEM_URI_PARSE_ADV(len + 1);
  }

  // consume //
  if (n < 2 || uri_str[0] != '/' || uri_str[1] != '/') {
    fprintf(ugemerr, "Expected '//' following scheme: in %s\n", start_uri);
    goto fail;
  }
  UGEM_URI_PARSE_ADV(2);

  // host:port
  {
    unsigned int hostport_len =
        ugem_tok_until(uri_str, '/', UGEM_TOK_OR_END, n);

    if (hostport_len == 0) {
      fprintf(ugemerr, "Uri does not conta a host: %s\n", start_uri);
      goto fail;
    }

    // host
    // optional :
    unsigned int host_len = ugem_tok_until(uri_str, ':', 0, hostport_len);
    // if this is 0 we do not have a port and can just use hostportlen
    if (host_len == 0) {
      uri.host = ugem_strndup(uri_str, hostport_len);
      uri.port = default_port;
      UGEM_URI_PARSE_ADV(hostport_len);
    } else {
      uri.host = ugem_strndup(uri_str, host_len);
      UGEM_URI_PARSE_ADV(host_len);

      int port_len = (int)hostport_len - (int)host_len;
      if (port_len <= 0 || port_len > 32) {
        fprintf(ugemerr, "Uri does not contain port after : was seen: %s:\n",
                start_uri);
        goto fail;
      }
      char port_str[64];
      strncpy(port_str, uri_str, n);

      // if : get port
      uri.port = (int)strtol(port_str, NULL, 10);

      UGEM_URI_PARSE_ADV(port_len);
    }
  }

  // optional / at the end
  if (uri_str[0] != '/') {
    uri.path = ugem_strndup("", 0);
    goto finish;
  }

  // consume the /
  UGEM_URI_PARSE_ADV(1);
  {
    // path string
    unsigned int pathfrag_len =
        ugem_tok_until(uri_str, '?', UGEM_TOK_OR_END, n);

    if (pathfrag_len == 0) {
      uri.path = ugem_strndup("", 0);
      goto finish;
    }

    // optional # for fragment
    unsigned int path_len = ugem_tok_until(uri_str, '#', 0, pathfrag_len);
    if (path_len == 0) {
      // we only have a path without a fragment
      path_len = pathfrag_len;
    }

    uri.path = ugem_malloc(path_len + 1);
    unsigned long path_res = ugem_uri_unescape(uri.path, uri_str, path_len);
    if (path_res) {
      fprintf(ugemerr, "Failed to unescape path in %s\n", start_uri);
      goto fail;
    }

    UGEM_URI_PARSE_ADV(path_len);

    int frag_len = (int)pathfrag_len - (int)path_len - 1;

    // if # get fragment
    if (frag_len > 0) {
      // consume #
      UGEM_URI_PARSE_ADV(1);
      uri.fragment = ugem_malloc(frag_len + 1);

      unsigned long frag_res =
          ugem_uri_unescape(uri.fragment, uri_str, frag_len);
      if (frag_res) {
        fprintf(ugemerr, "Failed to unescape fragment in %s\n", start_uri);
        goto fail;
      }

      UGEM_URI_PARSE_ADV(frag_len);
    }
  }

  // optional ?

  // if ? get key=value
  // repeat until end is not &

  if (n != 0) {
    fprintf(ugemerr, "Trailing tokens in %s: %ld bytes remaining\n", start_uri, n);
    goto fail;
  }

finish:
  uri.err = 0;
  return uri;

fail:
  return uri;
}

void ugem_uri_free(struct ugem_uri *uri) {
  if (uri->path) {
    ugem_free(uri->path);
  }

  if (uri->fragment) {
    ugem_free(uri->fragment);
  }

  ugem_free(uri->scheme);
  ugem_free(uri->host);

  // TODO: free rest of data
}

#undef UGEM_URI_PARSE_ADV

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
