#ifndef NET_H_
#define NET_H_

#include <arpa/inet.h>

int ugem_net_server_socket_init(const char *bind_addr, int port, sa_family_t family);
void ugem_net_socket_close(int socket);

// init a secure connection and return
// a context to the connection 
void* ugem_net_secure_ctx_init(void);

void ugem_net_secure_ctx_free(void *ctx);

// establish a connection context from an fd and the 
// secure connection context
void* ugem_net_secure_handshake(void *ctx, int fd);

void ugem_net_secure_disconnect(void *connection, int fd);

// writes data to the socket represented by ctx
// returns <= 0 on error or the amount of bytest written
long ugem_net_secure_write(void *connection, const char *data, unsigned long len);

long ugem_net_secure_read(void *connection, char *buf, unsigned long max);

#endif 
