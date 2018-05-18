CC = gcc
SOURCE_WEB_SERVER = webServerSource/webServerMain.c webServerSource/requestsParsing.c webServerSource/fdQueue.c webServerSource/readers.c webServerSource/socketManager.c
NAME_OF_EXECUTABLE_SERVER = myhhtpd
OBJECT = $(SOURCE_WEB_SERVER:.c=.o)
VALGRIND_FLAGS = --leak-check=yes --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes -v
SERVING_PORT = 8085
COMMAND_PORT = 9085
WEB_SERVER_THREADS = 10
ROOT_DIR = /home/mike/Desktop/threadPoolWebServer/webSites/
HOST = localhost
REQ_SITE = /site0/page0_4119.html

all: server compTests
	@echo Compile finished

#Webserver compile/running
server: $(SOURCE_WEB_SERVER) $(NAME_OF_EXECUTABLE_SERVER)
	$(CC) -g -O0 -Wall -o  $(NAME_OF_EXECUTABLE_SERVER) $(SOURCE_WEB_SERVER) -lm -pthread
	@echo Compiled web server

$(NAME_OF_EXECUTABLE_SERVER): $(OBJECT)
	$(CC) -g  -O0 $(OBJECT) -o $@ -lm -pthread

.c.o:
	$(CC) -c $< -o $@ -lm 

runServer:
	./$(NAME_OF_EXECUTABLE_SERVER) -p $(SERVING_PORT) -c $(COMMAND_PORT) -t $(WEB_SERVER_THREADS) -d $(ROOT_DIR)

runValgrindServer:
	valgrind $(VALGRIND_FLAGS) ./$(NAME_OF_EXECUTABLE_SERVER) -p $(SERVING_PORT) -c $(COMMAND_PORT) -t $(WEB_SERVER_THREADS) -d $(ROOT_DIR)

#Web server TESTS part
compTests:
	$(CC) -o webServerTests/shutdown webServerTests/shutdownTest.c
	$(CC) -o webServerTests/stats webServerTests/statsTest.c
	$(CC) -o webServerTests/siteReq webServerTests/servingTest.c
	@echo Compiled test programs in webServerTests directory

runShutdown:
	./webServerTests/shutdown $(HOST) $(COMMAND_PORT)

runStats:
	./webServerTests/stats $(HOST) $(COMMAND_PORT)

runSiteReq:
	./webServerTests/siteReq $(HOST) $(SERVING_PORT) $(REQ_SITE)


clean:
	rm -f $(NAME_OF_EXECUTABLE_SERVER)
	rm -f webServerSource/*.o
	rm -f webServerTests/shutdown
	rm -f webServerTests/siteReq
	rm -f webServerTests/stats
