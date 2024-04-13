#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ugem.h"
#include "net.h"
#include "uri.h"

#define TESTBEGIN(name) printf("[test '%s']\n", (name));
#define TESTEND(name) printf("[test '%s' ok]\n", (name));

#define assert_url_unescape(expect, expect_ret, escaped)                       \
  {                                                                            \
    char buf[1024];                                                            \
    int ret = ugem_uri_unescape(buf, escaped, strlen(escaped));                \
    printf("%s: rc = '%d' unescaped = '%s'\n", escaped, ret, buf);               \
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

int main(int arc, char **argv) {
  TESTBEGIN("ugem");

  url_unescape();

  TESTEND("ugem");

  return 0;
}
