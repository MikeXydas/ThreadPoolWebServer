#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>

#include "socketInterface.h"

int bindSockToPort(int port)
{
        struct sockaddr_in server;
        struct sockaddr * server_ptr = (struct sockaddr *) &server;
        int sock;

        if ((sock=socket(AF_INET, SOCK_STREAM, 0))<0) 
                perror("Failed to create server socket");

        int enable = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
                perror("setsockopt(SO_REUSEADDR) failed");
                
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(port); 
        memset(&server.sin_zero, 0, sizeof(server.sin_zero));

        if (bind(sock, server_ptr, sizeof(server)) < 0)
                perror("Failed to bind server socket");

        if (listen(sock,200)!=0) 
                perror("Failed to listen to server scoket");

        return sock;
}

int acceptConnection(int * fds, struct sockaddr *addr, socklen_t *addrlen, int * whichPort)
{
        fd_set readfds;
        int maxfd, fd;
        int status;

        FD_ZERO(&readfds);
        maxfd = -1;
        for (int i = 0; i < PORT_NUMB; i++) 
        {
                FD_SET(fds[i], &readfds);
                if (fds[i] > maxfd)
                maxfd = fds[i];
        }

        //printf("Selecting...\n");
        status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        //printf("Select returned\n");
        if (status < 0)
                return -1;
        fd = -1;
        for (int i = 0; i < PORT_NUMB; i++)
        {
                if (FD_ISSET(fds[i], &readfds)) 
                {
                        *whichPort = i;   
                        fd = fds[i];
                        break;
                }
        }
        if (fd == -1)
                return -1;
        else
                return accept(fd, addr, addrlen);
}

