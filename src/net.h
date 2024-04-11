#ifndef NET_H_
#define NET_H_

#include <arpa/inet.h>

int ugem_net_server_socket_init(int port, sa_family_t family);
void ugem_net_socket_close(int socket);

// init a secure connection and return
// a context to the connection 
void* ugem_net_secure_ctx_init(void);

void ugem_net_secure_ctx_free(void *ctx);

#endif 
