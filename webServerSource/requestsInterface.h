#ifndef REQUESTS_INTERFACE_H
#define REQUESTS_INTERFACE_H

#define MAX_PATH 300
#define WRONG_REQ_FORMAT NULL

#define BAD_REQUEST -1
#define SUCCESS 0
#define NO_PERMS 1
#define NO_FILE 2

#define LINES_ANWSER 8

#define NO_HOST_FIELD -1
#define HOST_FIELD_EXISTS 0

#define SERVER "Server: myhhtpd/1.0.0 (Ubuntu64)"
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_TYPE "Content-Type: text/html"
#define CONNECTION "Connection: Closed"

#define NO_PERMS_MSG "<html>Tying to access file but don't think I can make it.</html>"
#define NO_FILE_MSG "<html>Sorry dude, couldn't find this file.</html>"
#define BAD_REQUEST_MSG "<html> Request sent does not follow the http1.1 format.</html>"

extern char rootdir[200];

char * readRequest(char * req);
char * createAnwser(char * site, char * rootDir);
char * parseArrayToMsg(char ** array);

//Msg writing
void applyDate(char ** buf);
void applyContentLength(char ** buf, int length);
void applyContent(char ** buf, int fileAccess, FILE * fp, int length);
int hostFieldExists(char * header);

int digitsOfInt(unsigned long int integer);
int countCharacters(FILE * inputFile);


void printMsg(char ** array);

#endif
