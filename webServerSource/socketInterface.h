#ifndef SOCKET_INTERFACE_H
#define SOCKET_INTERFACE_H

#define PORT_NUMB 2
#define REQUEST_PORT 0
#define COMMAND_PORT 1


int bindSockToPort(int port);
int acceptConnection(int * fds, struct sockaddr *addr, socklen_t *addrlen, int * whichPort);

#endif