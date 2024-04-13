#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ugem.h"
#include "net.h"
#include "uri.h"

#define TESTBEGIN(name) printf("[test '%s']\n", (name));
#define TESTEND(name) printf("[test '%s' ok]\n\n", (name));

#define assert_url_unescape(expect, expect_ret, escaped)                       \
  {                                                                            \
    char buf[1024];                                                            \
    int ret = ugem_uri_unescape(buf, escaped, strlen(escaped));                \
    printf("%s: rc = '%d' unescaped = '%s'\n", escaped, ret, buf);             \
    assert(ret == expect_ret);                                                 \
    assert(strcmp(buf, expect) == 0);                                          \
  }

void url_unescape(void) {
  TESTBEGIN("url unescape");
  assert_url_unescape("test", 0, "test");
  assert_url_unescape("test with spaces", 0, "test%20with%20spaces");
  assert_url_unescape("", 12, "failedescape%");
  assert_url_unescape("", 12, "failedescape%1");
  assert_url_unescape("", 12, "failedescape%1Z");
  assert_url_unescape("", 12, "failedescape%Z1");
  TESTEND("url unescape");
}

#define assert_tok_until(expect, src, until, flags)                            \
  {                                                                            \
    int ret = ugem_tok_until(src, until, flags, strlen(src));                  \
    printf("%s: expect: %d parsed %d\n", src, expect, ret);                    \
    assert((expect) == ret);                                                   \
  }

void tok_until(void) {
  TESTBEGIN("tokuntil");

  assert_tok_until(4, "test:123", ':', 0);
  assert_tok_until(0, "test", ':', 0);
  assert_tok_until(4, "test", ':', UGEM_TOK_OR_END);

  TESTEND("tokuntil");
}

int main(int arc, char **argv) {
  TESTBEGIN("ugem");

  url_unescape();
  tok_until();

  TESTEND("ugem");

  return 0;
}
