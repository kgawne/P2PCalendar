//Kelly Gawne and Jack Magiera
//Calendar server
//Compile: make
//Usage: ./myCald <serverType> <portNum>

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
#include <fcntl.h>
#include <unistd.h>

#define DEBUG 1
#define MAXBUFFLEN 1024 //max nmber of bytes at once

int threaded = 0; //flag for threaded/iterative

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
		exit(1);
	}

	freeaddrinfo(serverInfo);

	return sockListen_fd;
}

void *thread_handler(void *sockfd){
 	//Get the socket descriptor
    int clientfd = *(int*)sockfd;

    //Fully connected! Yay!
    if (DEBUG) printf("Inside thread\n");
	//read in client data


	return 0;
}

void iterative_handler(void *sockfd){
	//get the socket descriptor
	int clientfd = *(int*)sockfd;
    if (DEBUG) printf("Inside iterative_handler\n");

    //Operate

}

int main(int argc, char* argv[]){ //remove arg input...?

	//Parse cmd line for server options
	if (argc != 3){
		puts("\t Please specify iterative or threaded style server");
		puts("\t\tex: ./myCald threaded <portNum>\tOR\t./myCald iterative <portNum");
		exit(1);
	}else{
		if(!strcmp(argv[1], "iterative")){
			threaded = 0;
		}else if (!strcmp(argv[1], "threaded")){
			threaded = 1;
		}else{
			puts("\t Please specify iterative or threaded style server");
			puts("\t\tex: ./myCald threaded <portNum>\tOR\t./myCald iterative <portNum");
			exit(1);		
		}
	}


	//Bind to port 9770
	char* portNum = argv[2];
	socklen_t sockListen_fd = bindToPort(portNum);
	int err = 0;

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
		
		//accept
		clientfd = accept(sockListen_fd, (struct sockaddr*)&client_addr, &clientAddrLen);
		if (DEBUG)
			printf("SERVER:  %s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		//Handle connection
		if (threaded){
			pthread_t thread_id;
			if( pthread_create( &thread_id , NULL, thread_handler, (void*) &clientfd) < 0){
				perror("couldn't create thread");
				return 1;
			}			
		}else{
			iterative_handler((void*) &clientfd);
		}

	}

	//Clean up. Shouldn't end up here.
	close(sockListen_fd);
	return 0; 
}