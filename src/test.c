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
  assert_tok_until_n(expect, src, until, flags, strlen(src))

#define assert_tok_until_n(expect, src, until, flags, n)                       \
  {                                                                            \
    int ret = ugem_tok_until(src, until, flags, n);                            \
    printf("%s: expect: %d parsed %d\n", src, expect, ret);                    \
    assert((expect) == ret);                                                   \
  }

void tok_until(void) {
  TESTBEGIN("tokuntil");

  assert_tok_until(4, "test:123", ':', 0);
  assert_tok_until(0, "test", ':', 0);
  assert_tok_until(4, "test", ':', UGEM_TOK_OR_END);
  assert_tok_until_n(0, "test", ':', 0, strlen("test") - 1);
  assert_tok_until_n(3, "test", ':', UGEM_TOK_OR_END, strlen("test") - 1);

  TESTEND("tokuntil");
}

void print_uri(struct ugem_uri *uri) {
  printf("error: %d -> %s://%s:%d/", uri->err, uri->scheme, uri->host, uri->port);

  if (uri->path) {
    printf("%s", uri->path);
  }

  if (uri->fragment) {
    printf("#%s", uri->fragment);
  }

  if (uri->query_len > 0 && uri->query) {
    printf("?");
  }

  for (int i = 0; i < uri->query_len && uri->query; i++) {
    printf("%s=%s", uri->query[i].key, uri->query[i].value);
    if (i + 1 < uri->query_len) {
      printf("&");
    }
  }
}

#define assert_uri_parse(expect, src)                                          \
  {                                                                            \
    struct ugem_uri ret = ugem_uri_parse(src, 123, strlen(src));               \
    printf("%s: expect: '", src);                                              \
    print_uri(&(expect));                                                      \
    printf("' got: '");                                                        \
    print_uri(&ret);                                                           \
    printf("'\n");                                                             \
    assert((expect).err == ret.err);                                           \
    assert((expect).port == ret.port);                                         \
    assert((expect).query_len == ret.query_len);                               \
    assert(strcmp((expect).scheme, ret.scheme) == 0);                          \
    assert(strcmp((expect).host, ret.host) == 0);                              \
    if ((expect).path || ret.path) {                                           \
      assert(strcmp((expect).path, ret.path) == 0);                            \
    }                                                                          \
    if ((expect).fragment || ret.fragment) {                                   \
      assert(strcmp((expect).fragment, ret.fragment) == 0);                    \
    }                                                                          \
    for (int i = 0; i < (expect).query_len; i++) {                             \
      assert(strcmp((expect).query[i].key, ret.query[i].key) == 0);            \
      if ((expect).query[i].value || ret.query[i].value) {                       \
        assert(strcmp((expect).query[i].value, ret.query[i].value) == 0);      \
      }                                                                        \
    }                                                                          \
    ugem_uri_free(&ret);                                                       \
  }

void uri_parse(void) {
  TESTBEGIN("uti parse");

  // no file paht but /
  struct ugem_uri uri1 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = ""};
  assert_uri_parse(uri1, "gemini://test.local/");

  // no /
  struct ugem_uri uri6 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = ""};
  assert_uri_parse(uri6, "gemini://test.local");

  // file path
  struct ugem_uri uri2 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1"};
  assert_uri_parse(uri2, "gemini://test.local/file/path/1");

  // file path with space
  struct ugem_uri uri3 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file path/1"};
  assert_uri_parse(uri3, "gemini://test.local/file%20path/1");

  // fragment
  struct ugem_uri uri4 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .fragment = "fragment"};
  assert_uri_parse(uri4, "gemini://test.local/file/path/1#fragment");

  // fragment with spaces
  struct ugem_uri uri5 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .fragment = "fragment with space"};
  assert_uri_parse(uri5,
                   "gemini://test.local/file/path/1#fragment%20with%20space");

  // key=value
  struct ugem_query query7[] = {{.key = "key1", .value = "value1"}};
  struct ugem_uri uri7 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .query = query7,
                          .query_len = 1};
  assert_uri_parse(uri7, "gemini://test.local/file/path/1?key1=value1");

  struct ugem_query query8[] = {{.key = "key1", .value = "value1"},
                                {.key = "key2", .value = "value2"}};
  struct ugem_uri uri8 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .query = query8,
                          .query_len = 2};
  assert_uri_parse(uri8,
                   "gemini://test.local/file/path/1?key1=value1&key2=value2");

  struct ugem_query query9[] = {{.key = "key1", .value = NULL},
                                {.key = "key2", .value = NULL}};
  struct ugem_uri uri9 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .query = query9,
                          .query_len = 2};
  assert_uri_parse(uri9, "gemini://test.local/file/path/1?key1&key2=");

  struct ugem_uri uri10 = {.err = 0,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .query_len = 0};
  assert_uri_parse(uri10, "gemini://test.local/file/path/1?");

  // unexpected end
  struct ugem_uri uri11= {.err = 1,
                          .scheme = "gemini",
                          .host = "test.local",
                          .port = 123,
                          .path = "file/path/1",
                          .fragment = NULL};
  assert_uri_parse(uri11, "gemini://test.local/file/path/1#");

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
