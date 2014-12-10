//Kelly Gawne and Jack Magiera
//Thermo sensor server
//Compile: make
//Usage: ./thermd

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <pthread.h>
#include <time.h>
#include "sensor_data.h"

#define DEBUG 1
#define MAXBUFFLEN 1024 //max nmber of bytes at once

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

//return a sockaddr for IPv4 or IPv6
void *get_sockaddr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){ //if 4
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr); // if 6
}

int bindToPort(char* portNum){ //bind to that port. Returns sockListen_fd
	int sockListen_fd;
	struct addrinfo hints, *serverInfo, *p;
	int err;
	int yes = 1;
	int numbytes;
	char buf[MAXBUFFLEN];
	socklen_t addr_len;
	char s [INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; //ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((err = getaddrinfo(NULL, portNum, &hints, &serverInfo)) != 0){
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
	    puts("\tHint: Did you enter a valid port number?");
        return 1;	
	}

		//Loop through available connections and bind to first socket
	for (p = serverInfo; p!= NULL; p = p->ai_next){
		if ((sockListen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("Server: socket");
			continue;
		}

 		if (setsockopt(sockListen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        //Bind 
        if (bind(sockListen_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockListen_fd);
            perror("Server: bind");
            continue;
        }
        break;
	}

	if (p == NULL) {
		puts("Failed to bind with socket.");
		return 1;
	}

	freeaddrinfo(serverInfo);

	return sockListen_fd;
}

void getFilename(char filename[25]){
	time_t timer;
    char buffer[25];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 25, "%Y:%m:%d%H:%M:%S", tm_info);
    strcpy(filename, buffer);
}

void *thread_handler(void *sockfd){
 	//Get the socket descriptor
    int clientfd = *(int*)sockfd;

    //Fully connected! Yay!
    if (DEBUG) printf("Inside thread\n");
	//read in client data

    //if (packet.action_req == 0){

	//Get Timestamp
	time_t timer;
	struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer); //store reading in log file (ex: g09_2014_11_student02)

	//Get filepath
	char filepath[256];
	char filename[25];
	strftime(filename, 25, "g09_%Y_%m_", tm_info); 
	//strcat(filename, packet1.host_name); //add hostname
	if (DEBUG) printf("Writing to filename '%s': ",filename);
	sprintf(filepath, "/var/log/therm/error/%s", filename);

	pthread_mutex_lock(&mutex1);
	//Open Log
	FILE *filePtr = fopen(filepath, "w+");

	char buffer[256];
	strftime(buffer, 256, "%Y %m %d %H %M ", tm_info); //add sensor readings
	// char sensorbuf[25];
	// sprintf(sensorbuf, "%f.2 %f.2\n", packet1.reading_val, packet2.reading_val);
	// strcat(buffer, sensorbuf);
	if (DEBUG) printf("'%s'", buffer);
	fseek(filePtr, 0, SEEK_END);
	fwrite(buffer, sizeof(char), strlen(buffer), filePtr);

	fclose(filePtr);

	pthread_mutex_unlock(&mutex1);

	return 0;
}

int main(int argc, char* argv[]){ //remove arg input...?

	char* portNum = "9770";

	//Bind to port 9770
	int sockListen_fd = bindToPort(portNum);
	int err;

	//Listen for connections
	if ( listen(sockListen_fd, 20) != 0 )	{
		perror("socket--listen");
		exit(err);
	}

	//When new connections arrive
	while(1){
		if (DEBUG) puts("SERVER: waiting for connections...");

		int clientfd;
		struct sockaddr_in client_addr;
		int clientAddrLen = sizeof(client_addr);
		pthread_t thread_id;
		
		//accept
		clientfd = accept(sockListen_fd, (struct sockaddr*)&client_addr, &clientAddrLen);
		if (DEBUG)
			printf("SERVER:  %s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		//create new thread
		if( pthread_create( &thread_id , NULL, thread_handler, (void*) &clientfd) < 0){
			perror("couldn't create thread");
			return 1;
		}

	}

	//Clean up. Shouldn't end up here.
	close(sockListen_fd);
	return 0; 
}