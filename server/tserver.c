//Kelly Gawne and Jack Magiera
//Calendar server
//Compile: make
//Usage: ./myCald <serverType> <portNum>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define DEBUG 1
#define MAXBUFLEN 1024 //max nmber of bytes at once

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

time_t convertDateTime(char* inDate, char* inTime){
	time_t rawtime = 0;
	struct tm temp = {0,0,0, 0,0,0, 0,0,0};

	//Convert date
	temp.tm_mon = (inDate[0] - '0') * 10 + (inDate[1] - '0') - 1;
	temp.tm_mday = (inDate[2] - '0') * 10 + (inDate[3] - '0');
	temp.tm_year = 100 + (inDate[4] - '0')*10 + (inDate[5] - '0');

	//Convert time
	temp.tm_hour = (inTime[0] - '0')*10 + (inTime[1] - '0');
	temp.tm_min = (inTime[2] - '0')*10 + (inTime[3] - '0');

	char buffer[25];
	strftime(buffer, 25, "%Y:%m:%d%H:%M:%S", &temp);
	if(DEBUG) printf("CONVERTED TIME: %s\n", buffer);
	rawtime = mktime(&temp);
	return rawtime;
}

void getEventAttribute(xmlDocPtr doc, xmlNodePtr event, char* attribute, char* value){ //Given a ptr to the event, return the given attribute
	//for jack to implement
	//get root
    xmlNodePtr cur = event;
    char buffer[40];

	//traverse tree until attribute found
	while (cur != NULL){
		if (xmlStrcmp(cur->name, (xmlChar *) attribute) == 0){
			if (DEBUG) printf("%s\n", cur->name);
			strcpy(buffer, xmlNodeListGetString(doc, cur->xmlChildrenNode,1));
			break;
		} 
		cur = cur->next;
	}

	//return attribute value
	strcpy(value, buffer);
}

time_t getStart(xmlDocPtr doc, xmlNodePtr event){
	char* inDate, *inTime;
	getEventAttribute(doc, event, "startDate", inDate);
	getEventAttribute(doc, event, "startTime", inTime);

	return convertDateTime (inDate, inTime);
}

time_t getEnd(xmlDocPtr doc, xmlNodePtr event){
	char *inDate,*inTime, *length;
	getEventAttribute(doc, event, "startDate", inDate);
	getEventAttribute(doc, event, "startTime", inTime);
	getEventAttribute(doc, event, "length", length);

	time_t startTime = convertDateTime (inDate, inTime);
	int duration_sec = atof(length) * 60 * 60;
	time_t rawEnd = startTime + duration_sec;

	return rawEnd;
}

int conflictExists(xmlDocPtr doc, xmlNodePtr event1, xmlNodePtr event2){
	if (getStart(doc, event1) < getEnd(doc, event2)) return 1; //doc1 starts during doc2
	else if (getStart(doc, event2) < getEnd(doc, event1)) return 1; //doc2 starts during doc1
	else return 0;
}


void *thread_handler(void *sockfd){
 	//Get the socket descriptor
    int clientfd = *(int*)sockfd;


    xmlChar *buffer = malloc(sizeof(xmlChar) * MAXBUFLEN) ;

    xmlDocPtr in_command, saved_cal;
    xmlNodePtr cur;
    xmlNodePtr saved_root;
    xmlNodePtr root;
    xmlChar debug_dump[1000];
    xmlChar command[40];
    char calendarPath[40];
    xmlBufferPtr tempEvent = xmlBufferCreate();
    char tempEvents[1000];
    char calendarName[40];
    int debug_size;

    struct stat st = {0};


    //Fully connected! Yay!
    if (DEBUG) printf("Inside thread\n");

	//read in client packet size
	uint32_t Nxml_size, xml_size;
	recv( clientfd, &Nxml_size, sizeof(uint32_t), 0);
	xml_size = ntohl(Nxml_size);
	if(DEBUG) printf("server: received xmlSize %u\n", (short unsigned) xml_size);

	//Read in client packet data
	bzero(buffer, MAXBUFLEN);
	recv (clientfd, buffer, sizeof(xmlChar) * MAXBUFLEN, 0);
	if(DEBUG) printf("server: received xml----%s\n", buffer);

	in_command = xmlParseDoc(buffer);
	cur = xmlDocGetRootElement(in_command)->xmlChildrenNode->next;
	if(DEBUG) printf("parsing buffer\n");
	strcpy(command,xmlNodeListGetString(in_command, cur->xmlChildrenNode,1));
	if(DEBUG) printf("server: command = %s\n", command);

	if (xmlStrcmp(command,(xmlChar *) "add") == 0){
		// begin add
		if (stat("calendars", &st) == -1){
			mkdir("calendars", 0777);
			printf("Made new calendar folder.\n");
		}
		strcpy(calendarPath,(char *)"calendars/");
		cur = xmlDocGetRootElement(in_command)->xmlChildrenNode->xmlChildrenNode;
		
		while (cur != NULL){
			if (xmlStrcmp(cur->name, (xmlChar *)"calendar") == 0){
				if (DEBUG) printf("%s\n", cur->name);
				strcpy(calendarName,xmlNodeListGetString(in_command, cur->xmlChildrenNode,1));
				strcat(calendarPath, calendarName);
			} 
			cur = cur->next;
		}
		strcat(calendarPath,".xml");
		
		if (stat (calendarPath, &st) == 0){
			saved_cal = xmlParseFile(calendarPath);
		} else {
			
			saved_cal = xmlNewDoc("1.0");
			saved_root = xmlNewNode(NULL,calendarName);
			xmlDocSetRootElement(saved_cal,saved_root);	
		}
		saved_root = xmlDocGetRootElement(saved_cal);
		root = saved_root;
		cur = xmlDocGetRootElement(in_command)->xmlChildrenNode;
		xmlAddChild(root,cur);
		
		xmlSaveFormatFile(calendarPath, saved_cal, 1);

		// TODO: RESPONSE
	} else if (xmlStrcmp(command,(xmlChar *) "remove") == 0) {

		if (stat("calendars", &st) == -1){
			mkdir("calendars", 0777);
			printf("Made new calendar folder.\n");
			// TODO: Just exit
		}
		strcpy(calendarPath,(char *)"calendars/");
		cur = xmlDocGetRootElement(in_command)->xmlChildrenNode->xmlChildrenNode;
		
		while (cur != NULL){
			if (xmlStrcmp(cur->name, (xmlChar *)"calendar") == 0){
				if (DEBUG) printf("%s\n", cur->name);
				strcpy(calendarName,xmlNodeListGetString(in_command, cur->xmlChildrenNode,1));
				strcat(calendarPath, calendarName);
			} 
			cur = cur->next;
		}
		strcat(calendarPath,".xml");
		if (stat (calendarPath, &st) == 0){
			// exit
		} else {
			
			saved_cal = xmlNewDoc("1.0");
			saved_root = xmlNewNode(NULL,calendarName);
			xmlDocSetRootElement(saved_cal,saved_root);	
		}
		saved_root = xmlDocGetRootElement(saved_cal)->xmlChildrenNode;
		cur = xmlDocGetRootElement(in_command)->xmlChildrenNode;
		while (saved_root != NULL ){
			if (xmlStrcmp(saved_root->name, "event")==0){
				cur = saved_root->xmlChildrenNode;
				while (cur != NULL ){
					printf("%s\n", cur->name);
					cur=cur->next;
				}
			}
		 	saved_root=saved_root->next;
		}

	} else if (xmlStrcmp(command,(xmlChar *) "get") == 0) {
// 	GET goes here

	} else if (xmlStrcmp(command,(xmlChar *) "getslow") == 0){
	// GETSLOW goes here
		//find relevant events and store in array
		//xmlChar** events;

		//Send number of relevent events
		uint16_t NnumEvents, numEvents = 0;
		NnumEvents = htons(numEvents);
		//send(clientfd, NnumEvents, sizeof(uint16_t), 0);

		if (numEvents != 0){
			//Loop through relevant events
			int i = 0;
			for(i = 0; i < numEvents; i++){
				//uint32_t xml_size, Nxml_size;
				//define xml_size
				//Nxml_size = htonl(xml_size);
				//xmlChar* doc = events[i];
				//send(clientfd, Nxml_size, sizeof(uint32_t), 0); //send size
				//send(clientfd, doc, xml_size, 0); //send event
				sleep(1);
			}			
		}

	}

	return 0;
}

// int getIntFromTree(xmlDocPtr doc, xmlNodePtr cur, char *type){
// 	while (cur != NULL){
// 		if (xmlStrcmp(cur->name, (xmlChar *)type) == 0){
// 			return(atoi(xmlNodeListGetString(in_command, cur->xmlChildrenNode,1)))
// 		} 
// 		cur = cur->next;
// 	}
// 	return -1;
// }

// bool conflicts(xmlDocPtr doc, xmlNodePtr cur, int d, int m, int y, int h, int m){
// 	if ()
// }

void iterative_handler(void *sockfd){
	//get the socket descriptor
	int clientfd = *(int*)sockfd;
    if (DEBUG) printf("Inside iterative_handler\n");

 	//read in client packet size
	uint32_t Nxml_size, xml_size;
	recv( clientfd, &Nxml_size, sizeof(uint32_t), 0);
	xml_size = ntohl(Nxml_size);
	if(DEBUG) printf("server: received xmlSize %u\n", (short unsigned) xml_size);

}

int main(int argc, char* argv[]){

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


	//Bind to given port
	char* portNum = argv[2];
	int sockListen_fd = bindToPort(portNum);
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
		socklen_t clientAddrLen = sizeof(client_addr);
		
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