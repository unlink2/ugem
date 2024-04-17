#ifndef UGEM_H__
#define UGEM_H__

#include <stdio.h>
#include <sys/socket.h>

#define UGEM_PATH_MAX 1024
#define UGEM_NET_BUF_MAX 2048
#define UGEM_PATH_SEP '/'

#define UGEM_GEMINI_LF "\r\n"

#define UGEM_ENV_KEY "UGEM_KEY"
#define UGEM_ENV_CERT "URGME_CERT"
#define UGEM_ENV_ROOT_DIR "UGEM_ROOT_DIR"

#define UGEM_DEFAULT_KEY "./server.key"
#define UGEM_DEFAULT_CERT "./server.cert"
#define UGEM_DEFAULT_ROOT_DIR "."

#define UGEM_TEXT_GEMINI "text/gemini"

#define UGEM_DEFAULT_PORT 1965;
#define UGEM_DEFAULT_SA_FAMILY AF_INET

#define UGEM_SHOULD_LOG(level) (ugemcfg.verbose >= (level))

#define ugem_malloc(size) malloc((size))
#define ugem_calloc(n, size) calloc(n, size)
#define ugem_realloc(ptr, n) realloc(ptr, n)
#define ugem_free(ptr) free(ptr)

#define ugem_log(f, ...)                                                       \
  {                                                                            \
    char buf[128];                                                             \
    time_t rawtime = 0;                                                        \
    struct tm *timeinfo = NULL;                                                \
    time(&rawtime);                                                            \
    timeinfo = localtime(&rawtime);                                            \
    strftime(buf, sizeof(buf), "%c", timeinfo);                                \
    fprintf((f), "<%s> ", buf);                                                \
    fprintf((f), __VA_ARGS__);                                                 \
  }

enum UGEM_LOG_LEVEL {
  UGEM_CRITICAL = 0,
  UGEM_ERROR = 1,
  UGEM_WARNING = 2,
  UGEM_INFO = 3,
  UGEM_DEBUG = 4
};

extern FILE *ugemin;
extern FILE *ugemout;
extern FILE *ugemerr;

struct ugem {
  int server_listening;

  int server_fd;
  int next_trace;
};

struct ugem_host_config {
  const char *root_path;
  const char *host;
  int all;
};

struct ugem_request {
  const char *payload;
  long payload_len;

  const char *src_addr;
  int src_port;

  int trace;
};

enum ugem_status {
  UGEM_INPUT = 10,
  UGEM_INPUT_SENSITIVE = 11,

  UGEM_SUCCESS = 20,

  UGEM_REDIRECT_TEMP = 30,
  UGEM_REDIRECT_PERM = 31,

  UGEM_TMP_FAIL_UNSPECIFIED = 40,
  UGEM_TMP_FAIL_SERVER_UNAVAIL = 41,
  UGEM_TMP_FAIL_CGI_ERR = 42,
  UGEM_TMP_FAIL_SLOW_DOWN = 44,

  UGEM_FAIL_NOT_FOUND = 51,
  UGEM_FAIL_GONE = 52,
  UGEM_FAIL_PROXY_REFUSED = 53,
  UGEM_FAIL_BAD_REQUEST = 59,

  UGEM_CERT_REQUIRED = 60,
  UGEM_CERT_NOT_AUTH = 61,
  UGEM_CERT_INVALID = 62
};

struct ugem_config {
  char **argv;
  int argc;

  int verbose;

  const char *key_path;
  const char *cert_path;

  struct ugem_host_config hostcfg;

  int port;
  sa_family_t sa_family;
};

extern struct ugem ugem;
extern struct ugem_config ugemcfg;

void ugem_init(struct ugem_config cfg);
void ugem_print_payload(FILE *f, const char *buf, long read);

// checks if path can be used
// according to the following rules:
// a path may not start with /
// a path may not contain ../
// a path may not end in /..
int ugem_is_path_valid(const char *path, unsigned long n);

// joins 2 paths into dst.
// dst must be n bytest long
int ugem_path_join(char *dst, const char *p1, const char *p2, char sep,
                   unsigned long n);

int ugem_gettrace(void);

struct ugem_config ugem_cfg_init(void);

int ugem_main(struct ugem_config cfg);

#endif
