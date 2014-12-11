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
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define DEBUG 0
#define MAXBUFLEN 2048

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

char* getAttribute(xmlDocPtr doc, char* attribute){
	//for jack to implement
	//get root
	xmlDocPtr in_command, saved_cal;
    xmlNodePtr cur;

	//traverse tree until attribute found

	//return attribute value
}

time_t getStart(xmlDocPtr doc){
	char* inDate = getAttribute(doc, "startDate");
	char* inTime = getAttribute(doc, "startTime");

	return convertDateTime (inDate, inTime);
}

time_t getEnd(xmlDocPtr doc){
	char* inDate = getAttribute(doc, "startDate");
	char* inTime = getAttribute(doc, "startTime");
	char* length = getAttribute(doc, "length");

	time_t startTime = convertDateTime (inDate, inTime);
	int duration_sec = atof(length) * 60 * 60;
	time_t rawEnd = startTime + duration_sec;

	return rawEnd;
}

int main(int argc, char *argv[]){

	xmlDocPtr toSend;
	xmlNodePtr cur, root;
	uint32_t xml_size;
	xmlChar *doc;

	//Open connection to server NOTE: hard-coded to student02 in order to 
	if (argc < 3 || argc > 7) {
		printf("argc: %d (needs to be >= 3)\n", argc);
		return 1;
	}

	char* cmd = argv[2];

	if (strcmp(cmd,"add") == 0){
		if (argc != 7){
			printf("Syntax for add: ./myCal <calendar> add <mmddyy> <hhmm> <length> <name>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 || strlen(argv[4]) != 4){
			printf("Syntax for <date>: mmddyy %d\nSyntax for <time>: hhmm %d\n", strlen(argv[3]), strlen(argv[4]));
			return 1;
		}

		char* inDate = argv[3];
		char* inTime = argv[4];

		time_t rawStart = convertDateTime( inDate, inTime);
		if(DEBUG) printf("CONVERTED TIME returned: %d\n", rawStart);
		printf("Current time = %s", ctime(&rawStart));
		int duration_sec = atof(argv[5]) * 60 * 60;
		time_t rawEnd = rawStart + duration_sec;

		if (rawEnd < rawStart){
			printf("Invalid date entered. End of event cannot preceed start.\n");
			return 1;
		}

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"message");
		xmlDocSetRootElement(toSend,root);
		cur = xmlNewTextChild(root, NULL, "event", NULL);
		xmlNewTextChild(cur, NULL, "startDate", argv[3]);
		xmlNewTextChild(cur, NULL, "startTime", argv[4]);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);
		xmlNewTextChild(cur, NULL, "length", argv[5]);
		xmlNewTextChild(cur, NULL, "name", argv[6]);
		xmlAddSibling(cur,xmlNewDocNode(toSend,NULL,"command","add"));

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(cmd,"remove") == 0){

		if (argc != 5){
			printf("Syntax for remove: ./myCal <calendar> remove <mmddyy> <hhmm>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 || strlen(argv[4]) != 4){
			printf("Syntax for <date>: mmddyy %d\nSyntax for <time>: hhmm %d\n", strlen(argv[3]), strlen(argv[4]));
			return 1;
		}

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"message");
		xmlDocSetRootElement(toSend,root);

		cur = xmlNewTextChild(root, NULL, "event", NULL);
		xmlNewTextChild(cur, NULL, "startDate", argv[3]);
		xmlNewTextChild(cur, NULL, "startTime", argv[4]);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);
		xmlAddSibling(cur,xmlNewDocNode(toSend,NULL,"command","remove"));

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(cmd,"update") == 0) {

		if (argc != 7){
			printf("Syntax for update: ./myCal <calendar> update <mmddyy> <hhmm> <length> <name>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 || strlen(argv[4]) != 4){
			printf("Syntax for <date>: mmddyy %d\nSyntax for <time>: hhmm %d\n", strlen(argv[3]), strlen(argv[4]));
			return 1;
		}

		char* inDate = argv[3];
		char* inTime = argv[4];

		time_t rawStart = convertDateTime( inDate, inTime);
		int duration_sec = atof(argv[5]) * 60 * 60;
		time_t rawEnd = rawStart + duration_sec;

		if (rawEnd < rawStart){
			printf("Invalid date entered. End of event cannot preceed start.\n");
			return 1;
		}

		char startStr[25], endStr[25];
		sprintf(startStr, "%d", &rawStart);
		sprintf(endStr, "%d", &rawEnd);

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"message");
		xmlDocSetRootElement(toSend,root);
		cur = xmlNewTextChild(root, NULL, "event", NULL);
		xmlNewTextChild(cur, NULL, "startDate", argv[3]);
		xmlNewTextChild(cur, NULL, "startTime", argv[4]);
		xmlNewTextChild(cur, NULL, "length", argv[5]);
		xmlNewTextChild(cur, NULL, "name", argv[6]);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);
		xmlAddSibling(cur,xmlNewDocNode(toSend,NULL,"command","update"));

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(cmd,"get") == 0) {

		if (argc != 4){
			printf("Syntax for get: ./myCal <calendar> get <mmddyy>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 ){
			printf("Syntax for <date>: mmddyy %d\n", strlen(argv[3]));
			return 1;
		}

		char* inDate = argv[3];
		char* inTime = "0000";

		time_t rawStart = convertDateTime( inDate, inTime);

		char startStr[25];
		sprintf(startStr, "%d", &rawStart);

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"message");
		xmlDocSetRootElement(toSend,root);
		cur = xmlNewTextChild(root, NULL, "event", NULL);
		xmlNewTextChild(cur, NULL, "startDate", argv[3]);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);
		xmlAddSibling(cur,xmlNewDocNode(toSend,NULL,"command","get"));

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(cmd,"getslow") == 0) {

		if (argc != 3){
			printf("Syntax for get: ./myCal <calendar> getslow\n");
			return 1;
		}
		
		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"event");
		xmlDocSetRootElement(toSend,root);
		cur = xmlNewTextChild(root, NULL, "event", NULL);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);
		xmlAddSibling(cur,xmlNewDocNode(toSend,NULL,"command","getslow"));
		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);
	} else {
		printf("%s not a supported command.\n",cmd);
		return 1;

	} 

	//Socket set up
	int serverfd;
	struct addrinfo hints, *res;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//Read in server info
	FILE *serverFile = fopen(".myCal", "r");
	if (serverFile == NULL){
		printf(".myCal file cannot be opened.\n");
		exit(1);
	}

	char line[128];
	char hostname[25], portnum[25];
	fgets( line, 128, serverFile);
	sscanf(line, "%s %s", hostname, portnum);

	fclose(serverFile);
	if(DEBUG) printf("Connecting to '%s' on port '%s'\n", hostname, portnum);	

	getaddrinfo(hostname, portnum, &hints, &res); 

	if ((serverfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol)) == -1){
		printf("Socket Failure\n");
		exit(1);
	}

	//Connect to server
	char local_host[1024];
	gethostname(local_host, 1023);

	if (connect(serverfd, res->ai_addr, res->ai_addrlen) < 0){
		printf("Could not connect to server '%s' on port %s.\n", hostname, portnum);
		printf("\tHINT: Is the server running here?\n");
		exit(1);
	}


	//Send data size to server
	uint32_t Nxml_size = htonl(xml_size);
	send(serverfd, &Nxml_size, sizeof(Nxml_size), 0);
	if(DEBUG) printf("client: sent xmlSize--%d\n", xml_size);

	//Send data to server
	send(serverfd, doc, xml_size, 0);

	if (DEBUG) printf("client: sending xml----%s\n",doc);

//-----------------------------------------------------------------------------

	//Handle server response...
	xmlChar *buffer = malloc(sizeof(xmlChar) * MAXBUFLEN);


	if ( !strcmp( cmd, "add") || !strcmp(cmd, "remove") || !strcmp(cmd, "update")){

		bzero(buffer, MAXBUFLEN);
		recv (serverfd, buffer, sizeof(xmlChar) * MAXBUFLEN, 0);

		//Print response
		printf("%s\n", buffer);
		//???

	}else if (strcmp(cmd, "get") || strcmp(cmd, "getslow")){
		//Get number of events
		uint16_t numEvents, NnumEvents;
		recv( serverfd, &NnumEvents, sizeof(uint16_t), 0);
		numEvents = ntohs(NnumEvents);
		// if(DEBUG) printf("server: received num: %d\n", numEvents);
		if( numEvents == 0){
			printf("There are no applicable events\n");
		}else{
			int i = 0;
			for ( i = 0; i < numEvents; i++){
				//Get event data
				bzero(buffer, MAXBUFLEN);
				recv (serverfd, buffer, sizeof(char) * MAXBUFLEN, 0);

				//Print event
				printf("%s\n", buffer);	
				//???

			}			
		}
	}

	free(buffer);
	return 0;
}