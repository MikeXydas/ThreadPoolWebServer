#ifndef FD_QUEUE_INTERFACE_H
#define FD_QUEUE_INTERFACE_H

#include <pthread.h>
#include <unistd.h>

#define EMPTY_QUEUE -1

typedef struct QueueNode queueNode;

typedef struct HeadOfQueue
{
        int numbOfNodes;
        queueNode * firstNode;
} headOfQueue;

struct QueueNode
{
        int fd;
        queueNode * nextNode;
};

//Queue that will store all the fds that have been accepted in the serving_port
extern headOfQueue * fdQueue;

//Global mutex that protects the queue
extern pthread_mutex_t queueMutex;

//We will only allow one reader or one writer (since we are using a queue)
extern pthread_cond_t cond_write;
extern pthread_cond_t cond_read;
extern int writing;
extern int reading;
extern int queueSize;

//Queue synched push/pop
void writeOnQueue(int fd);
int readFromQueue();

//Queue struct basic functions
headOfQueue * createQueue();
queueNode * createNode(int fd);
void insertInQueue(int fd, headOfQueue * head);
int popFromQueue(headOfQueue * head);
void deleteQueue(headOfQueue * head);
void deleteQueueNode(queueNode * node);

//Thread read/write synchronisation
void enterWrite();
void exitWrite();
void enterRead();
void exitRead();

#endif