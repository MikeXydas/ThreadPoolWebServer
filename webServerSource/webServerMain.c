#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>
#include <getopt.h>

#include "requestsInterface.h"
#include "fdQueueInterface.h"
#include "readersInterface.h"
#include "socketInterface.h"

int main(int argc, char *argv[])
{
        gettimeofday(&begin, NULL);

        printf("\n>>> Server is booting...\n");

        int numbOfThreads = -1 , reqPort = -1, commPort = -1, err;
        rootdir[0] = '!';

        int opt;
        while((opt = getopt(argc, argv, "p:c:t:d:")) != -1)
        {
                switch(opt)
                {
                        case 'd':
                                strcpy(rootdir, optarg);
                                break;
                        case 'p':
                                reqPort = atoi(optarg);
                                break;
                        case 'c':
                                commPort = atoi(optarg);
                                break;
                        case 't':
                                numbOfThreads = atoi(optarg);
                                break;
                        case '?':
                                printf("Unknown argument, exiting...\n");
                                return 1;
                }
        }

        if((numbOfThreads == -1) || (reqPort == -1) || (commPort == -1) || (rootdir[0] == '!'))
        {
                printf("Wrong arguments, correct format: ./myhttpd -p <serving_port> -c <command_port> -t <numb_of_threads> -d <root_dir>\n");
                return 1;
        }


        //Mutexes/conds initialisation
        pthread_mutex_init(&queueMutex, 0);
        pthread_cond_init(&cond_read, 0);
        pthread_cond_init(&cond_write, 0);

        pthread_mutex_init(&statsMut, 0);

        //Queue initialisation
        fdQueue = createQueue();
        

        //Pool creation
        pthread_t * readers;
        if ((readers = malloc(numbOfThreads * sizeof(pthread_t))) == NULL) 
        {
                perror("Malloc of threads failed");  
                exit(1); 
        }
        
        for (int whichThread = 0 ; whichThread < numbOfThreads; whichThread++) 
        {
                if ((err = pthread_create(readers + whichThread, NULL, reader, 0))) {
                        perror2("Failed to create reader thread", err);   exit(1);} 
        }

        //Bind ports
        int * fds = (int *) malloc(PORT_NUMB * sizeof(int));
        fds[REQUEST_PORT] = bindSockToPort(reqPort);
        fds[COMMAND_PORT] = bindSockToPort(commPort);

        //Wait to accept
        
        int incomingFd = 0;
        int whichPort = -1;

        printf(">>> Serving port = %d | Command port = %d | Number of threads = %d\n", reqPort, commPort, numbOfThreads);
        printf("\n>>> Server is ready\n\n");

        while(shutdownReceived == 0)
        {
                struct sockaddr_in client;
                socklen_t clientlen = 0;
                struct sockaddr *clientptr=(struct sockaddr *)&client;
                incomingFd = acceptConnection(fds, clientptr, &clientlen, &whichPort);
                if(incomingFd == -1)
                        perror("Failed to accept");
                
                if(whichPort == REQUEST_PORT)
                {
                        writeOnQueue(incomingFd);
                }
                else if(whichPort == COMMAND_PORT)
                {
                        commandReader(incomingFd);
                        close(incomingFd);
                }
                else
                        perror("Unexpected whichPort");

        }

        for (int whichThread = 0 ; whichThread < numbOfThreads; whichThread++) 
        {
                if ((err = pthread_join(*(readers + whichThread), NULL))) 
                {
                        perror2("Failed to pthread_join", err); 
                        exit(1); 
                }         
        }

        free(readers);
        free(fds);
        deleteQueue(fdQueue);
        pthread_cond_destroy(&cond_read);
        pthread_cond_destroy(&cond_write);
        pthread_mutex_destroy(&queueMutex);
        pthread_mutex_destroy(&statsMut);
        
        return 0;
}



