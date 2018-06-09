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
        char buf[2000];
        struct sockaddr_in server;
        struct sockaddr *serverptr = (struct sockaddr*)&server;
        struct hostent *rem;
        if (argc != 4) 
        {
                printf("Please give host name, port number and requested site\n");
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

        //GET request creation
        strcpy(buf, "GET ");
        strcat(buf, argv[3]);
        strcat(buf, " HTTP/1.1\r\n");
        strcat(buf, "Host: localhost\r\n\r\n");
        
        if(write(sock, buf, strlen(buf)) < 0)
                perror("Client failed to write on socket");

        //take the header
        int headerPointer = 0;
        while(1)
        {
                if (read(sock, buf + headerPointer, 1) < 0)
                        perror_exit("read");

                if(headerPointer >= 3)
                {
                        if((buf[headerPointer - 3] == '\r') && (buf[headerPointer - 2] == '\n') && (buf[headerPointer - 1] == '\r') && (buf[headerPointer] == '\n'))
                        {
                                buf[headerPointer + 1] = '\0';
                                break;
                        }
                }
                headerPointer++;
        }

        char * content = (char *) malloc((findContentLength(buf) + 1) * sizeof(char));
        int contentPointer;
        char c;
        while(read(sock, &c, 1) > 0)
        {
                content[contentPointer] = c;
                contentPointer++;
        }
        content[contentPointer] = '\0';

        printf("Content: %s", content);

        close(sock);         
}

int findContentLength(char * buf)
{
        char length[9];
        char * contentStart = strstr(buf, "Content-Length:");
        int whichCharBuf = 0, whichCharLength = 0, retLength;
        
        while(contentStart[whichCharBuf] != ' ')
                whichCharBuf++;
        whichCharBuf++;

        while(contentStart[whichCharBuf] != '\r')
        {
                length[whichCharLength] = contentStart[whichCharBuf];
                whichCharBuf++;
                whichCharLength++;
        }
        length[whichCharLength] = '\0';
        if((retLength = atoi(length)) <= 0)
                printf("Failed to read content-length\n");

        return retLength;

}


void perror_exit(char *message)
{
        perror(message);
        exit(EXIT_FAILURE);
}