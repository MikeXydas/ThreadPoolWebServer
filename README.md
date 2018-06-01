# Thread pool Web Server
  
  
## Introduction

This is a basic server that will answer simple `HTTP/1.1` requests. The main thread accepts the connect from the client and a reader thread will read the request and answer providing the whole HTML file requested (or any file that exists in the `<server_directory>`).
  
  
## Compile

I suggest going through the `Makefile` as you read this part  

- `make all` / `make` : Will compile the web server files creating the  `myhttpd` executable and the server test programs (More on them later)  
- `make server` : Will compile only the web server  
- `make compTests` : Will compile only the test programs  
  
  
## Run server

The server constantly listens on 2 ports. The `serving_port` is for answering the site requests, the  `command_port` is for answering/executing basic commands. Implemented commands are STATS which returns time_running | pages_sent | bytes_sent and SHUTDOWN which begins the shutdown sequence of the server.  
  
Also, the server needs a directory which will search for the files requested. In my repo, the `webSites` directory has these files.  

**Necessary flags:**  
      1. `-p <serving_port>`  
      2. `-c <command_port>`  
      3. `-t <thread_numb>` Do not put an unnecessarily big number as it will end up slowing the server due to synchronization overhead  
      4. `-d <server_directory>`  
      
For example a run command of my repo would be:  
      `./myhttpd -p 8085 -c 9095 -t 4 -d /home/users/.../threadPoolWebServer/webSites/`  
      (Care, on the directory we need the last `/`)  
  
In the `Makefile` you can also see a runServer rule. You can use that too, if you adjust properly the `SERVING_PORT`, `COMMAND_PORT`, `WEB_SERVER_THREADS`, `ROOT_DIR`.

## Running tests on the server

The simplest test would be to open your browser and go to:  
    `http://localhost:<serving_port>/<path_to_the_site_from_server_directory>`  
     e.x.: `http://localhost:8085/site2/page2_1667.html/`  
As you can see thw webServer will not serve any page. Just only the ones being saved in the root_directory.  
Or you can use the siteReq test program located in the `webServerTests` directory.  
  
**Test programs execution:**  
    1. siteReq: `./siteReq <host> <serving_port> <file_requested>`  
    2. stats: `./stats <host> <command_port>`  
    3. shutdown `./shutdown <host> <command_port>`  
    
I suggest as a host to put localhost. For remote access, although possible the firewall will block it.

These executions can also be called by the commands `make runSiteReq`, `make runStats`, `make runShutdown`.    
Care when you adjust the `REQ_SITE` variable in the `Makefile`.  
  
  
## webcreator.sh

A simple shell script which will create our `<server_directory>`.  
It creates `w` websites and each website has `p` pages.  
  
`./webcreator.sh <base_directory> <text_file> <w> <p>`  
e.x. `./webcreator.sh webSites text 5 5`  
  
1. `<base_directory>` : A file preferably located in the starting directory of this repo. **Must already exist before executing the shell.**  
2. `<text_file>` : A sample text file that we use to create the HTML files. My repository has the `Twenty Thousand Leagues under the Sea by Jules Verne` http://www.gutenberg.org/ebooks/164   
(If you want to put your text file, in order for the script to work you should put something with 10.000 lines or more).  
3. `<w>` : Number of websites created  
4. `<p>` : Number of pages each website has


