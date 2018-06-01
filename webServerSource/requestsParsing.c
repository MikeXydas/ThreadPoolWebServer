#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "requestsInterface.h"
#include "readersInterface.h"

char rootdir[200];

char * readRequest(char * req)
{
        int msgPointer = 4;
        //int readWholeMsg = 4;

        char path[MAX_PATH];

        //check that it is a get request
        if((req[0] != 'G') || (req[1] != 'E') || (req[2] != 'T') || (req[3] != ' ') || (req[4] == ' '))
                return WRONG_REQ_FORMAT;

        //check that the ending is correct
        int reqLength = strlen(req);
        int correctFormat = 0;

        //Formal \r\n endings
        if((req[reqLength - 4] == '\r') && (req[reqLength - 3] == '\n') && (req[reqLength - 2] == '\r') && (req[reqLength - 1] == '\n'))
                correctFormat = 1;

        //Only \n endings
        if((req[reqLength - 2] == '\n') && (req[reqLength - 1] == '\n'))
                correctFormat = 1;

        if(correctFormat == 0)
                return WRONG_REQ_FORMAT;

        //Check HOST field exists
        if(hostFieldExists(req) == NO_HOST_FIELD)
                return WRONG_REQ_FORMAT;

        while(req[msgPointer] != ' ')
        {
                path[msgPointer - 4] = req[msgPointer];
                msgPointer++;
        }
        path[msgPointer - 4] = '\0';

        char * retPath = (char *) malloc(msgPointer * sizeof(char));

        strcpy(retPath, path);

        return retPath;             
}


char * createAnwser(char * site, char * rootDir)
{
        char path[MAX_PATH];
        char result[20];
        int fileAccess = -1;
        FILE * fp;
        if(site == NULL)
        {
                fileAccess = BAD_REQUEST;
                strcpy(result, "Bad Request");
        }
        else
        {
                strcpy(path, rootDir);
                strcat(path, site);


                fp = fopen(path, "r");


                if(fp != NULL)
                {
                        fileAccess = SUCCESS;
                        strcpy(result, "200 OK");
                }
                else if(errno==EACCES)
                {
                        fileAccess=NO_PERMS;
                        strcpy(result, "403 Forbidden");
                }
                else if(errno==ENOENT)
                {
                        fileAccess=NO_FILE;
                        strcpy(result, "404 Not Found");
                }
                else
                        perror("UNEXPECTED ERROR WHEN OPENING FILE");
        }
        free(site);
        
        char ** arrayAnwser = (char **) malloc(LINES_ANWSER * sizeof(char *));
 
        //1st line
        arrayAnwser[0] = (char *) malloc((9 + strlen(result) + 1) * sizeof(char));
        strcpy(arrayAnwser[0], "HTTP/1.1 ");
        strcat(arrayAnwser[0], result);

        //2nd line
        applyDate(arrayAnwser);

        //3rd line
        arrayAnwser[2] = (char *) malloc((strlen(SERVER) + 1) * sizeof(char));
        strcpy(arrayAnwser[2], SERVER);

        //4th line
        int length = -1;
        if(fileAccess == SUCCESS)
        {
                length = countCharacters(fp) + 1;

                pthread_mutex_lock(&statsMut);
                totalBytes += length;
                totalPages++;
                pthread_mutex_unlock(&statsMut);
        }
        else if(fileAccess == NO_PERMS)
                length = strlen(NO_PERMS_MSG);
        else if(fileAccess == NO_FILE)
                length = strlen(NO_FILE_MSG);
        else
                length = strlen(BAD_REQUEST_MSG);

        //if(fileAccess == SUCCESS)
        //        applyContentLength(arrayAnwser, length);
        //else
        applyContentLength(arrayAnwser, length);

        //5th line
        arrayAnwser[4] = (char *) malloc((strlen(CONTENT_TYPE) + 1) * sizeof(char));
        strcpy(arrayAnwser[4], CONTENT_TYPE);

        //6th line
        arrayAnwser[5] = (char *) malloc((strlen(CONNECTION) + 1) * sizeof(char));
        strcpy(arrayAnwser[5], CONNECTION);

        //7th line is blank

        //8th line
        applyContent(arrayAnwser, fileAccess, fp, length);

        //printMsg(arrayAnwser);

        if(fileAccess == SUCCESS)
                fclose(fp);

        char * msg = parseArrayToMsg(arrayAnwser);

        for(int whichLine = 0; whichLine < LINES_ANWSER; whichLine++)
        {
                if(whichLine != 6)
                        free(arrayAnwser[whichLine]);
        }
        free(arrayAnwser);

        return msg;

}

int hostFieldExists(char * header)
{
        char * hostStart = strstr(header, "Host: ");
        if(hostStart == NULL)
                return NO_HOST_FIELD;

        if(hostStart[6] == ' ' || hostStart[6] == '\r' || hostStart[6] == '\n')
                return NO_HOST_FIELD;

        return HOST_FIELD_EXISTS;
        
}

char * parseArrayToMsg(char ** array)
{
        int msgLength = 0;

        for(int whichLine = 0; whichLine < LINES_ANWSER; whichLine++)
        {
                if(whichLine != 6)
                        msgLength += strlen(array[whichLine]);
                msgLength += 2;
        }

        char * msg = (char *) malloc((msgLength + 1) * sizeof(char));

        strcpy(msg, array[0]);
        strcat(msg, "\r\n");

        for(int whichLine = 1; whichLine < LINES_ANWSER; whichLine++)
        {
                if(whichLine != 6)
                        strcat(msg, array[whichLine]);
                strcat(msg, "\r\n");
        }

        return msg;
}

void applyDate(char ** array)
{
        time_t timer;
        char timeBuffer[50];
        struct tm * tm_info;

        time(&timer);
        tm_info = localtime(&timer);

        strftime(timeBuffer, 50, "%c %Z", tm_info);

        array[1] = (char *) malloc((6 + strlen(timeBuffer) + 1) * sizeof(char));

        strcpy(array[1], "Date: ");
        strcat(array[1], timeBuffer);
}

void applyContentLength(char ** array, int length)
{
        char * sLength = (char *) malloc((digitsOfInt(length) + 1) * sizeof(char));
        sprintf(sLength, "%d", length);

        array[3] = (char *) malloc((strlen(sLength) + strlen(CONTENT_LENGTH) + 1) * sizeof(char));
        strcpy(array[3], CONTENT_LENGTH);
        strcat(array[3], sLength);

        free(sLength);
}

int digitsOfInt(unsigned long int integer)
{
        int count = 1;
        while((integer = integer/10) > 0)
                count++;
        return count;
}

void applyContent(char ** array, int fileAccess, FILE * fp, int length)
{
        if(fileAccess == SUCCESS)
        {
                int chars = countCharacters(fp);
                array[7] = (char *) malloc((chars + 1) * sizeof(char));
                fread(array[7], chars, sizeof(char), fp);
                //for(int whichChar = 0; whichChar < chars; whichChar++)
                //        array[7][whichChar] = getc(fp);
                array[7][chars] = '\0';
        }
        else if(fileAccess == NO_PERMS)
        {
                array[7] = (char *) malloc(length * sizeof(char));
                strcpy(array[7], NO_PERMS_MSG);
        }
        else if(fileAccess == NO_FILE)
        {
                array[7] = (char *) malloc(length * sizeof(char));
                strcpy(array[7], NO_FILE_MSG);
        }
        else
        {
                array[7] = (char *) malloc(length * sizeof(char));
                strcpy(array[7], BAD_REQUEST_MSG);
        }
}

int countCharacters(FILE * inputFile)
{
        fseek(inputFile, 0, SEEK_SET);
        int tempCharacters = 0;
        int c;
        while((c = getc(inputFile)) != EOF)
        {
                tempCharacters++;
        }

        fseek(inputFile, 0, SEEK_SET);
        return tempCharacters;
}

//TO BE REMOVED
void printMsg(char ** array)
{
        for(int whichLine = 0; whichLine < LINES_ANWSER; whichLine++)
        {
                if(whichLine == 6)
                        continue;
                printf("Line %d:%s\n", whichLine, array[whichLine]);
        }
}