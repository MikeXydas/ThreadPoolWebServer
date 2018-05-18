#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>          
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int findContentLength(char * buf);
void perror_exit(char *message);

void main(int argc, char *argv[]) {

        
        int port, sock, i;
        char buf[300];
        struct sockaddr_in server;
        struct sockaddr *serverptr = (struct sockaddr*)&server;
        struct hostent *rem;
        if (argc != 3) 
        {
                printf("Please give host name and port number\n");
                exit(1);
        }
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                perror_exit("socket");
        if ((rem = gethostbyname(argv[1])) == NULL) 
        {
                herror("gethostbyname"); 
                exit(1);
        }
        port = atoi(argv[2]);
        server.sin_family = AF_INET;       
        memcpy(&server.sin_addr, rem->h_addr_list[0], rem->h_length);
        server.sin_port = htons(port);         

        if (connect(sock, serverptr, sizeof(server)) < 0)
                perror_exit("connect");
        printf("Connecting to %s port %d\n", argv[1], port);
        
        strcpy(buf, "STATS");

        if(write(sock, buf, strlen(buf)) < 0)
                perror("Client failed to write on socket");

        //take the header
        int headerPointer = 0;
        while(1)
        {
                if (read(sock, buf + headerPointer, 1) < 0)
                        perror_exit("read");

                if(buf[headerPointer] == '!')
                {
                        buf[headerPointer + 1] = '\0';
                        break;
                }
                headerPointer++;
        }

        char * tok = strtok(buf, "#");
        printf("Time = %s seconds |", tok);

        tok = strtok(NULL, "#");
        printf(" Pages = %s |", tok);

        tok = strtok(NULL, "!");
        printf(" Bytes = %s\n", tok);

        close(sock);         
}



void perror_exit(char *message)
{
        perror(message);
        exit(EXIT_FAILURE);
}