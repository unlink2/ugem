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

void print_uri(struct ugem_uri *uri) {
  printf("%s://%s:%d/%s", uri->scheme, uri->host, uri->port, uri->path);

  if (uri->fragment) {
    printf("#%s", uri->fragment);
  }

  if (uri->query_len > 0 && uri->query) {
    puts("?");
  }

  for (int i = 0; i < uri->query_len && uri->query; i++) {
    printf("%s=%s", uri->query[i].key, uri->query[i].value);
  }
}

#define assert_uri_parse(expect, src)                                          \
  {                                                                            \
    struct ugem_uri ret = ugem_uri_parse(src, 123, strlen(src));               \
    printf("%s: expect: '", src);                                               \
    print_uri(&(expect));                                                      \
    printf("' got: '");                                                          \
    print_uri(&ret);                                                           \
    printf("'\n");                                                              \
    assert((expect).err == ret.err);                                           \
    assert((expect).port == ret.port);                                         \
    assert((expect).query_len == ret.query_len);                               \
    assert(strcmp((expect).scheme, ret.scheme) == 0);                          \
    assert(strcmp((expect).host, ret.host) == 0);                              \
    assert(strcmp((expect).path, ret.path) == 0);                              \
    assert(strcmp((expect).fragment, ret.fragment) == 0);                      \
    for (int i = 0; i < (expect).query_len; i++) {                             \
      assert(strcmp((expect).query[i].key, ret.query[i].key) == 0);            \
      assert(strcmp((expect).query[i].value, ret.query[i].value) == 0);        \
    }                                                                          \
    ugem_uri_free(&ret);                                                       \
  }

void uri_parse(void) {
  TESTBEGIN("uti parse");

  struct ugem_uri uri1 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = ""};
  assert_uri_parse(uri1, "gemini://test.local/");

  TESTEND("uri parse");
}

int main(int arc, char **argv) {
  struct ugem_config cfg = ugem_cfg_init();
  ugem_init(cfg);

  TESTBEGIN("ugem");

  url_unescape();
  tok_until();
  uri_parse();

  TESTEND("ugem");

  return 0;
}
