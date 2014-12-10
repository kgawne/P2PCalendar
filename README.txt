TEAM MEMBERS
Kelly Gawne and Jack Magiera

FILES INCLUDED
	README.txt
	thermo.h
	CLIENT
		tclient.c
		makefile
	SERVER
		tserver.c
		makefile



CODE
We start with our simple TCP client and server. Then we added the threading in the server. Next we modified Jeff Sadowski's code to read from the sensors in the client. We then designed a data structure to store the sensor info in. This sensor data would be sent to the server in a struct. The server would receive and parse the struct, make the necessary additions to log files. 

USAGE
Compile the server my navigating to proj2/server and running "make", run with "./thermd"
Compile the client by navigating to proj2/client and running "make", run with "./therm"


DESIGN DECISIONS
The server is currently hard-coded to run on student02 as there were no specifications in the document as to how the command line arguments should be formatted. The server is run with ./thermd and the client with ./therm

If there are no sensors for that host, no reading is recorded.


