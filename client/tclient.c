//Kelly D Gawne and Jack Magiera
// Calendar client. Takes in cmd line input
//compile: make
//run: ./myCal <arguments>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define DEBUG 1

int main(int argc, char *argv[]){

	//Open connection to server  


	//Socket set up
	int socket_fd;
	struct addrinfo hints, *res;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo("student02.cse.nd.edu","9770", &hints, &res); 
	//NOTE: hard-coded to student00 in order to match argument constraints

	if ((socket_fd = socket(res->ai_family,res->ai_socktype,res->ai_protocol)) == -1){
		printf("Socket Failure\n");
		exit(1);
	}

	//Connect to server
	char local_host[1024];
	gethostname(local_host, 1023);

	if (connect(socket_fd, res->ai_addr, res->ai_addrlen) < 0){
		printf("connection failure\n");
		exit(1);
	}

	//Send data structure to server
	//send(socket_fd, &(sensor_readings[i]), sizeof(struct sensor_data), 0);
	//if (DEBUG) printf("Sent %f\n",sensor_readings[i].reading_val);

	//Handle server response...

	return 0;
}