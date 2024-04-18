#include "net.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "ugem.h"
#include <fcntl.h>

int ugem_net_server_socket_init(const char *bind_addr, int port,
                                sa_family_t family) {
  int s = -1;
  struct sockaddr *addr = NULL;
  struct sockaddr_in addr4;
  struct sockaddr_in6 addr6;

  unsigned long addr_len = 0;

  switch (family) {
  case AF_INET:
    addr4.sin_family = family;
    addr4.sin_port = htons(port);
    if (ugemcfg.bind_addr) {
      inet_pton(AF_INET, ugemcfg.bind_addr, &(addr4.sin_addr));
    } else {
      addr4.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    addr = (struct sockaddr *)&addr4;
    addr_len = sizeof(addr4);
    break;
  case AF_INET6:
    addr6.sin6_family = family;
    addr6.sin6_port = htons(port);
    addr6.sin6_flowinfo = 0;

    if (ugemcfg.bind_addr) {
      inet_pton(AF_INET, ugemcfg.bind_addr, &(addr6.sin6_addr));
    } else {
      addr6.sin6_addr = in6addr_loopback;
    }

    addr = (struct sockaddr *)&addr6;
    addr_len = sizeof(addr6);
    break;
  default:
    fprintf(ugemerr, "Unsupported address family\n");
    return -1;
  }

  s = socket(family, SOCK_STREAM, 0);
  int option = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (s < 0) {
    fprintf(ugemerr, "Socket creation failed: %s\n", strerror(errno));

    return -1;
  }

  if (bind(s, (struct sockaddr *)addr, addr_len) < 0) {
    fprintf(ugemerr, "Unable to bind socket on port %d: %s\n", port,
            strerror(errno));
    return -1;
  }

  if (listen(s, 1) < 0) {
    fprintf(ugemerr, "Unable to listen: %s\n", strerror(errno));
    return -1;
  }

  // int flags = fcntl(s, F_GETFL);
  // fcntl(s, F_SETFL, flags | O_NONBLOCK);
  if (UGEM_SHOULD_LOG(UGEM_DEBUG)) {
    fprintf(ugemerr, "Opened socket %d\n", s);
  }

  return s;
}

void ugem_net_socket_close(int socket) {
  if (UGEM_SHOULD_LOG(UGEM_DEBUG)) {
    fprintf(ugemerr, "Closing socket %d\n", socket);
  }

  close(socket);
}

#ifdef UGEM_TEST

// unit test "mock" connection

void *ugem_net_secure_ctx_init(void) { return NULL; }

void ugem_net_secure_ctx_free(void *ctx) {}

void *ugem_net_secure_handshake(void *ctx, int fd) { return NULL; }

void ugem_net_secure_disconnect(void *connection, int fd) {}

long ugem_net_secure_write(void *connection, const char *data,
                           unsigned long len) {
  return 0;
}

long ugem_net_secure_read(void *connection, char *buf, unsigned long max) {
  return 0;
}

#else

#include <openssl/ssl.h>
#include <openssl/err.h>

// openssl implementation

void *ugem_net_secure_ctx_init(void) {
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    fprintf(ugemerr, "Unable to create ssl context\n");
    return NULL;
  }

  if (SSL_CTX_use_certificate_chain_file(ctx, ugemcfg.cert_path) <= 0) {
    fprintf(ugemerr, "Unable to load certificate chain '%s'\n",
            ugemcfg.cert_path);
    return NULL;
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, ugemcfg.key_path, SSL_FILETYPE_PEM) <=
      0) {
    fprintf(ugemerr, "Unable to load key '%s'\n", ugemcfg.key_path);
    return NULL;
  }

  return ctx;
}

void ugem_net_secure_ctx_free(void *ctx) {
  if (!ctx) {
    return;
  }

  SSL_CTX_free(ctx);
}

void *ugem_net_secure_handshake(void *ctx, int fd) {
  SSL *ssl = SSL_new(ctx);
  SSL_set_fd(ssl, fd);

  if (SSL_accept(ssl) <= 0) {
    fprintf(ugemerr, "Unable to establish ssl connection\n");
    return NULL;
  }

  return ssl;
}

void ugem_net_secure_disconnect(void *connection, int fd) {
  SSL_shutdown(connection);
  SSL_free(connection);
  close(fd);
}

long ugem_net_secure_read(void *connection, char *buf, unsigned long max) {
  long read = SSL_read(connection, buf, (int)max);

  return read;
}

long ugem_net_secure_write(void *connection, const char *data,
                           unsigned long len) {
  return SSL_write(connection, data, (int)len);
}

#endif
