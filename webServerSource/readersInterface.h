#ifndef READERS_INTERFACE_H
#define READERS_INTERFACE_H

#include <sys/time.h>                // for gettimeofday()

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

#define COMMAND_END "!"
#define CHANGE_FIELD "#"

#define COMMAND_MAX_SIZE 30
//Shows to the readerThreads that the must exit
extern int shutdownReceived;

extern struct timeval begin;

extern pthread_mutex_t statsMut;

extern int totalBytes;
extern int totalPages;

void * reader();
void commandReader(int fd);
char * statsMsg();

#endif