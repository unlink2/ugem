#include "uri.h"
#include "ugem.h"
#include <ctype.h>

struct ugem_uri ugem_uri_parse(const char *uri_str, unsigned long n) {
  struct ugem_uri uri;

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

unsigned long ugem_uri_unescape(char *dst, const char *src, unsigned long n) {
  if (n == 0) {
    return 0;
  }
  const char uri_escape_char = '%';

  unsigned long writeat = 0;
  unsigned long readat = 0;

  for (readat = 0; readat < n; readat++) {
    char c = src[readat];
    if (c == uri_escape_char) {
      if (readat + 3 >= n) {
        goto fail;
      }

      char hexstr[3] = { src[readat+1], src[readat+2], '\0'};
    
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

const char *ugem_uri_escape(const char *src, unsigned long n) {
  
}

void ugem_uri_free(struct ugem_uri *uri) {
}
