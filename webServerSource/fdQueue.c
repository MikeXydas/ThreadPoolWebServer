#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fdQueueInterface.h"
#include "readersInterface.h"

//Global initialisation

headOfQueue * fdQueue;
pthread_mutex_t queueMutex;

pthread_cond_t cond_write;
pthread_cond_t cond_read;
int writing = 0;
int reading = 0;
int queueSize = 0;


void writeOnQueue(int fd)
{
        enterWrite();
        insertInQueue(fd, fdQueue);
        //printf("I am thread %ld and I just pushed %d\n", pthread_self(), fd);
        exitWrite();
}

int readFromQueue()
{
        int retFd;
        enterRead();
        retFd = popFromQueue(fdQueue);
        //printf("I am thread %ld and I just popped %d\n", pthread_self(), retFd);
        exitRead();
        return retFd;
}



headOfQueue * createQueue()
{
        headOfQueue * retHead = (headOfQueue *) malloc(sizeof(headOfQueue));
        retHead->firstNode = NULL;
        retHead->numbOfNodes = 0;
        return retHead;
}

queueNode * createNode(int fd)
{
        queueNode * retNode = (queueNode *) malloc(sizeof(queueNode));
        retNode->fd = fd;
        retNode->nextNode = NULL;
        return retNode;
}

void insertInQueue(int fd, headOfQueue * head)
{
        if(head->numbOfNodes == 0)
        {
                head->firstNode = createNode(fd);
        }
        else
        {
                queueNode * temp = head->firstNode;
                while(temp->nextNode != NULL)
                        temp = temp->nextNode;
                temp->nextNode = createNode(fd);
        }
        head->numbOfNodes += 1;

}

int popFromQueue(headOfQueue * head)
{
        int fd;
        queueNode * tempNode = NULL;
        if(head->numbOfNodes == 0)
                return EMPTY_QUEUE;


        if(head->firstNode->nextNode != NULL)
                tempNode = head->firstNode->nextNode;

        fd = head->firstNode->fd;
        free(head->firstNode);
        head->firstNode = tempNode;

        head->numbOfNodes -= 1;

        return fd;
}

void deleteQueue(headOfQueue * head)
{
        deleteQueueNode(head->firstNode);
        free(head);
}

void deleteQueueNode(queueNode * node)
{
        if(node == NULL)
                return;
        if(node->nextNode != NULL)
                deleteQueueNode(node->nextNode);
        free(node);
}

void enterWrite()
{
        pthread_mutex_lock(&queueMutex);
        while((writing > 0) || (reading > 0))
        {
                pthread_cond_wait(&cond_write, &queueMutex);
        }

        writing++;
        queueSize++;

        pthread_mutex_unlock(&queueMutex);
}

void exitWrite()
{
        pthread_mutex_lock(&queueMutex);
        writing--;
        //Only one thread writes so we do not have someone to wake up
        pthread_cond_signal(&cond_read);
        pthread_mutex_unlock(&queueMutex);
}

void enterRead()
{
        pthread_mutex_lock(&queueMutex);
        if(shutdownReceived == 1)
        {
                        pthread_mutex_unlock(&queueMutex);
                        pthread_exit(0);
        }
        while((writing > 0) || (reading > 0) || (queueSize == 0))
        {
                if(shutdownReceived == 1)
                {
                        pthread_mutex_unlock(&queueMutex);
                        pthread_exit(0);
                }
                if(queueSize == 0)
                        pthread_cond_signal(&cond_write);
                pthread_cond_wait(&cond_read, &queueMutex);
        }

        reading++;
        queueSize--;

        pthread_mutex_unlock(&queueMutex);
}

void exitRead()
{
        pthread_mutex_lock(&queueMutex);
        reading--;
        pthread_cond_signal(&cond_read);
        pthread_cond_signal(&cond_write);
        pthread_mutex_unlock(&queueMutex);
        if(shutdownReceived == 1)
                        pthread_exit(0);
}

