#include "net.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>

int ugem_net_socket_init(int port, sa_family_t family) {
  int s = -1;
  struct sockaddr_in addr;
  addr.sin_family = family;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  s = socket(family, SOCK_STREAM, 0);

  if (s < 0) {
    perror("Socket creation failed!");

    return -1;
  }

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Unable to bind socket");
    return -1;
  }

  if (listen(s, 1) < 0) {
    perror("Unable to listen");
    return -1;
  }

  return s;
}

#ifdef UGEM_TEST

// unit test "mock" connection

#else

// openssl implementation

#endif
