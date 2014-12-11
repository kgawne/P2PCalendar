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

#define DEBUG 1

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

	if (strcmp(argv[2],"add") == 0){
		if (argc != 7){
			printf("Syntax for add: ./myCal <calendar> add <mmddyy> <hhmm> <length> <name>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 || strlen(argv[4]) != 4){
			printf("Syntax for <date>: mmddyy %d\nSyntax for <time>: hhmm %d\n", strlen(argv[3]), strlen(argv[4]));
			return 1;
		}
		char monthStr[3];
		char dayStr[3];
		char yearStr[3];
		char hourStr[3];
		char minStr[3];

		memcpy(monthStr, &(argv[3][0]),2);
		memcpy(dayStr, &(argv[3][2]), 2);
		memcpy(yearStr, &(argv[3][4]), 2);
		memcpy(hourStr, &argv[4][0],2);
		memcpy(minStr, &(argv[4][2]), 2);


		monthStr[2] = '\0';
		dayStr[2] = '\0';
		yearStr[2] = '\0';
		hourStr[2] = '\0';
		minStr[2] = '\0';

		int mon = atoi(monthStr);
		int yr = atoi(yearStr);
		int day = atoi(dayStr);
		int hr = atoi(hourStr);
		int min = atoi(minStr);

		int monthDays[13];
		monthDays[0] = 31;
		monthDays[1] = 31;
		monthDays[2] = 28;
		monthDays[3] = 31;
		monthDays[4] = 30;
		monthDays[5] = 31;
		monthDays[6] = 30;
		monthDays[7] = 31;
		monthDays[8] = 31;
		monthDays[9] = 30;
		monthDays[10] = 31;
		monthDays[11] = 30;
		monthDays[12] = 31;

		int end_mon = mon;
		int end_yr = yr;
		int end_day = day;
		int end_hr = hr;
		int end_min = min;

		if ( mon > 12 
			|| day > monthDays[mon]
			|| hr > 23
			|| min > 59){
			printf("Improper date entered.\n");
			return 1;
		}
		int lenMins = atof(argv[5]) * 60;
		end_min = min + lenMins;
		while (end_min > 59){
			end_hr++;
			end_min-=60;
		}
		while (end_hr>23){
			end_day++;
			end_hr-=24;
		}
		while (end_day>monthDays[end_mon]){
			end_day-=monthDays[end_mon];
			end_mon++;	
		}
		while (end_mon>12){
			end_yr++;
			end_mon-=12;
		}

		char endmonthStr[3];
		char enddayStr[3];
		char endyrStr[3];
		char endhrStr[3];
		char endminStr[3];

		sprintf(endmonthStr,"%02d",end_mon);
		sprintf(enddayStr,"%02d",end_day);
		sprintf(endyrStr,"%02d",end_yr);
		sprintf(endhrStr,"%02d",end_hr);
		sprintf(endminStr,"%02d",end_min);

		endmonthStr[2] = '\0';
		enddayStr[2] = '\0';
		endyrStr[2] = '\0';
		endhrStr[2] = '\0';
		endminStr[2] = '\0';

		// printf("%s/%s/%s %s:%s\n", monthStr, dayStr, yearStr, hourStr, minStr );
		// printf("%d/%d/%d %d:%d\n", end_mon, end_day, end_yr, end_hr, end_min );

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"add");
		xmlDocSetRootElement(toSend,root);
		cur = xmlDocGetRootElement(toSend);

		xmlNewTextChild(cur, NULL, "monthStr", monthStr);
		xmlNewTextChild(cur, NULL, "dayStr", dayStr);
		xmlNewTextChild(cur, NULL, "yearStr", yearStr);
		xmlNewTextChild(cur, NULL, "hourStr", hourStr);
		xmlNewTextChild(cur, NULL, "minStr", minStr);
		xmlNewTextChild(cur, NULL, "end_mon", endmonthStr);
		xmlNewTextChild(cur, NULL, "end_day", enddayStr);
		xmlNewTextChild(cur, NULL, "end_yr",endyrStr);
		xmlNewTextChild(cur, NULL, "end_hr",endhrStr);
		xmlNewTextChild(cur, NULL, "end_min", endminStr);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);
		xmlNewTextChild(cur, NULL, "length", argv[5]);
		xmlNewTextChild(cur, NULL, "name", argv[6]);

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(argv[2],"remove") == 0){

		if (argc != 5){
			printf("Syntax for remove: ./myCal <calendar> remove <mmddyy> <hhmm>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 || strlen(argv[4]) != 4){
			printf("Syntax for <date>: mmddyy %d\nSyntax for <time>: hhmm %d\n", strlen(argv[3]), strlen(argv[4]));
			return 1;
		}

		char monthStr[3];
		char dayStr[3];
		char yearStr[3];
		char hourStr[3];
		char minStr[3];

		memcpy(monthStr, &(argv[3][0]),2);
		memcpy(dayStr, &(argv[3][2]), 2);
		memcpy(yearStr, &(argv[3][4]), 2);
		memcpy(hourStr, &argv[4][0],2);
		memcpy(minStr, &(argv[4][2]), 2);

		monthStr[2] = '\0';
		dayStr[2] = '\0';
		yearStr[2] = '\0';
		hourStr[2] = '\0';
		minStr[2] = '\0';

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"remove");
		xmlDocSetRootElement(toSend,root);
		cur = xmlDocGetRootElement(toSend);
		xmlNewTextChild(cur, NULL, "monthStr", monthStr);
		xmlNewTextChild(cur, NULL, "dayStr", dayStr);
		xmlNewTextChild(cur, NULL, "yearStr", yearStr);
		xmlNewTextChild(cur, NULL, "hourStr", hourStr);
		xmlNewTextChild(cur, NULL, "minStr", minStr);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(argv[2],"update") == 0) {

		if (argc != 7){
			printf("Syntax for update: ./myCal <calendar> update <mmddyy> <hhmm> <length> <name>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 || strlen(argv[4]) != 4){
			printf("Syntax for <date>: mmddyy %d\nSyntax for <time>: hhmm %d\n", strlen(argv[3]), strlen(argv[4]));
			return 1;
		}

		char monthStr[3];
		char dayStr[3];
		char yearStr[3];
		char hourStr[3];
		char minStr[3];

		memcpy(monthStr, &(argv[3][0]),2);
		memcpy(dayStr, &(argv[3][2]), 2);
		memcpy(yearStr, &(argv[3][4]), 2);
		memcpy(hourStr, &argv[4][0],2);
		memcpy(minStr, &(argv[4][2]), 2);

		monthStr[2] = '\0';
		dayStr[2] = '\0';
		yearStr[2] = '\0';
		hourStr[2] = '\0';
		minStr[2] = '\0';

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"update");
		xmlDocSetRootElement(toSend,root);
		cur = xmlDocGetRootElement(toSend);
		xmlNewTextChild(cur, NULL, "monthStr", monthStr);
		xmlNewTextChild(cur, NULL, "dayStr", dayStr);
		xmlNewTextChild(cur, NULL, "yearStr", yearStr);
		xmlNewTextChild(cur, NULL, "hourStr", hourStr);
		xmlNewTextChild(cur, NULL, "minStr", minStr);
		xmlNewTextChild(cur, NULL, "length", argv[5]);
		xmlNewTextChild(cur, NULL, "name", argv[6]);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(argv[2],"get") == 0) {

		if (argc != 4){
			printf("Syntax for get: ./myCal <calendar> get <mmddyy>\n");
			return 1;
		}
		if (strlen(argv[3]) != 6 ){
			printf("Syntax for <date>: mmddyy %d\n", strlen(argv[3]));
			return 1;
		}

		char monthStr[3];
		char dayStr[3];
		char yearStr[3];

		memcpy(monthStr, &(argv[3][0]),2);
		memcpy(dayStr, &(argv[3][2]), 2);
		memcpy(yearStr, &(argv[3][4]), 2);

		monthStr[2] = '\0';
		dayStr[2] = '\0';
		yearStr[2] = '\0';

		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"get");
		xmlDocSetRootElement(toSend,root);
		cur = xmlDocGetRootElement(toSend);
		xmlNewTextChild(cur, NULL, "monthStr", monthStr);
		xmlNewTextChild(cur, NULL, "dayStr", dayStr);
		xmlNewTextChild(cur, NULL, "yearStr", yearStr);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);

	} else if (strcmp(argv[2],"getslow") == 0) {

		if (argc != 3){
			printf("Syntax for get: ./myCal <calendar> getslow\n");
			return 1;
		}

		
		toSend = xmlNewDoc("1.0");
		root = xmlNewNode(NULL,"getslow");
		xmlDocSetRootElement(toSend,root);
		cur = xmlDocGetRootElement(toSend);
		xmlNewTextChild(cur, NULL, "calendar", argv[1]);

		xmlDocDumpMemory(toSend,&doc,&xml_size);
		xmlSaveFormatFile("sending.xml", toSend, 1);
	} else {

		printf("%s not a supported command.\n",argv[2]);
		return 1;

	} 

	//Socket set up
	int socket_fd;
	struct addrinfo hints, *res;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo("student02.cse.nd.edu","9771", &hints, &res); 
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


	//Send data size to server
	uint32_t Nxml_size = htonl(xml_size);
	send(socket_fd, &Nxml_size, sizeof(Nxml_size), 0);
	if(DEBUG) printf("client: sent xmlSize--%d\n", xml_size);

	//Send data to server
	send(socket_fd, doc, xml_size, 0);

	if (DEBUG) printf("client: sending xml----%s\n",doc);

	//Handle server response...

	return 0;
}