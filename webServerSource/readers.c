#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "requestsInterface.h"
#include "fdQueueInterface.h"
#include "readersInterface.h"

int shutdownReceived = 0;
struct timeval begin;
pthread_mutex_t statsMut;

int totalBytes = 0;
int totalPages = 0;

void * reader()
{
        //Big enough that will store the whole header
        char headerBuf[2000] = {0};
        while(shutdownReceived == 0)
        {
                //Read the GET request and send anwser
                int bufferPointer = 0;
                int fd = readFromQueue();
                while(read(fd, headerBuf + bufferPointer, 1) > 0)
                {
                        if(bufferPointer >= 3)
                                if((headerBuf[bufferPointer - 3] == '\r') && (headerBuf[bufferPointer - 2] == '\n') && (headerBuf[bufferPointer - 1] == '\r') && (headerBuf[bufferPointer] == '\n'))
                                        break;
                        bufferPointer++;
                }
                headerBuf[bufferPointer + 1] = '\0';

                char *  anwser = createAnwser(readRequest(headerBuf), rootdir);

                int charactersServed = 0;
                while(charactersServed != strlen(anwser))
                {
                        int currentWrite = write(fd, &anwser[charactersServed], strlen(anwser) - charactersServed);
                        if(currentWrite < 0)
                                perror("Failed to write search result on socket");

                        charactersServed += currentWrite;
                }
                //if(write(fd, anwser, strlen(anwser)) < 0)
                //        perror("Write of anwser from request failed");

                printf("        >>>Thread %ld succesfully anwsered serving port request\n", pthread_self());

                free(anwser);
                close(fd);
        }
        pthread_exit(0);
}

void commandReader(int fd)
{
        char command[COMMAND_MAX_SIZE] = {0};

        int readSize = -1;
        if((readSize = read(fd, command, COMMAND_MAX_SIZE - 1)) < 0)
                perror("Failed to read command from command port");
        command[readSize] = '\0';


        if(strcmp(command, "STATS") == 0)
        {
                char * msg = statsMsg();
                if(write(fd, msg, strlen(msg)) < 0)
                        perror("Failed to write stats\n");

                free(msg);
        }
        else if(strcmp(command, "SHUTDOWN") == 0)
        {
                shutdownReceived = 1;
                pthread_cond_broadcast(&cond_read);
                printf("\n              >>> Received SHUTDOWN command. Server shutting down <<<\n");
        }
        else
        {
                char tempBuf[20];
                strcpy(tempBuf, "UNKNOWN COMMAND!");
                if(write(fd, tempBuf, strlen(tempBuf)) < 0)
                        perror("Failed to write unknown command\n");
        }
        
        if(shutdownReceived == 0)
                printf("        >>>Thread %ld succesfully anwsered command port request\n", pthread_self());    

}

char * statsMsg()
{
        struct timeval end;
        gettimeofday(&end, NULL);

        double time_spent = end.tv_sec - begin.tv_sec;
        int intTimeSpent = (int) time_spent;

        int timeDigits = digitsOfInt(intTimeSpent);
        char * sTime = (char *) malloc((timeDigits + 1) * sizeof(char));
        sprintf(sTime, "%d", intTimeSpent);

        pthread_mutex_lock(&statsMut);

        int bytesDigits = digitsOfInt(totalBytes);
        char * sBytes = (char *) malloc((bytesDigits + 1) * sizeof(char));
        sprintf(sBytes, "%d", totalBytes);

        int pagesDigits = digitsOfInt(totalPages);
        char * sPages = (char *) malloc((pagesDigits + 1) * sizeof(char));
        sprintf(sPages, "%d", totalPages);

        pthread_mutex_unlock(&statsMut);

        int msgLength = timeDigits + bytesDigits + pagesDigits + 2 * strlen(CHANGE_FIELD) + strlen(COMMAND_END) + 1;
        char * msg = (char *) malloc(msgLength * sizeof(char));

        strcpy(msg, sTime);
        strcat(msg, CHANGE_FIELD);

        strcat(msg, sPages);
        strcat(msg, CHANGE_FIELD);

        strcat(msg, sBytes);
        strcat(msg, COMMAND_END);

        free(sTime);
        free(sPages);
        free(sBytes);

        return msg;
}