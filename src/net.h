#ifndef NET_H_
#define NET_H_

#include <arpa/inet.h>

int ugem_net_socket_init(int port, sa_family_t family);
int ugem_net_socket_close(int socket);

void* ugem_connection_init(void);

int ugem_connection_free(void);

#endif 
