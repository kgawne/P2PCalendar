TEAM MEMBERS
Kelly Gawne and Jack Magiera

FILES INCLUDED
	README.txt
	CLIENT
		tclient.c
		makefile
	SERVER
		tserver.c
		makefile
	
LAUNCH INSTRUCTIONS
	

	Navigate to the server directory. 
	Run make. 
	Initiate the server with the command './mycald <serverType> <portNum>', e.g. './mycald threaded 9770'

	Navigate to ../client.
	If necessary, alter the .myCal file to appropriate specs. No spaces.
		<hostname>
		<portnum>
	Run make.
	Run the client with the appropriate arguments for your operation

EXTRA FEATURE

	Cloud Storage: for our extra feature, we implemented cloud storage. All of our users' events are safely backed up on the server for future use.


EVALUATION

	ITERATIVE


	MULTI-THREADED
