/* timeserv.c - a socket-based time of day server
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netdb.h>
#include  <time.h>
#include  <strings.h>

//added for IP string conversion and lookup: - DBrooks
	#define _OPEN_SYS_SOCK_IPV6
	#include <arpa/inet.h>
	#include <string.h>
	#include <stdbool.h> 

#define   PORTNUM  13000   /* our time service phone number */
#define   HOSTLEN  256
#define   oops(msg)      { perror(msg) ; exit(1) ; }

//helper functions added:
	void printNetworkInfo(struct sockaddr_in *SockStruct, char *Type); //print server address and port - DBrooks
	bool isAllowedToConnect(char *ClientIP); //will check for the file in the dir for an allowed list - DBrooks

int main(int ac, char *av[])
{
	struct  sockaddr_in   saddr;   	/* build our address here */	
	struct	hostent		*hp;   		/* this is part of our    */
	char	hostname[HOSTLEN];     	/* address 	         */
	int		sock_id,sock_fd;       		/* line id, file desc     */
	FILE	*sock_fp;              	/* use socket as stream   */
	char    *ctime();              	/* convert secs to string */
	time_t  thetime;               	/* the time we report     */

	
	printf("\nServer code for timerServer C.N.N. Project:\n");


	/*
	* Step 1: ask kernel for a socket
	*/
	printf("Step 1 started...asking kernel for socket.\n"); // - added by DBrooks
		sock_id = socket( PF_INET, SOCK_STREAM, 0 );    /* get a socket */
		if ( sock_id == -1 ) 
			oops( "socket" );
		printf("\tsocket access was given by kernel...\n");
	printf("Step 1 ended...\n\n"); // - added by DBrooks


	/*
	* Step 2: bind address to socket.  Address is host,port
	*/
	printf("Step 2 started...binding socket to PID port Addr.\n"); // - added by DBrooks
		bzero( (void *)&saddr, sizeof(saddr) ); /* clear out struct     */

		gethostname( hostname, HOSTLEN );       /* where am I ?         */
		hp = gethostbyname( hostname );         /* get info about host  */
		
		/* fill in host part    */
			bcopy( (void *)hp->h_addr, (void *)&saddr.sin_addr, hp->h_length);
			saddr.sin_port = htons(PORTNUM);        /* fill in socket port  */
			saddr.sin_family = AF_INET ;            /* fill in addr family  */
		if ( bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0 )
	    	   oops( "bind" );
		printf("\tsocket bounded with port, assigned name to socket using sys call\n"); // - added by DBrooks
	printf("Step 2 ended...\n\n"); // - added by DBrooks

    
	/*
	* Step 3: allow incoming calls with Qsize=1 on socket
	*/
	printf("Step 3 started...assign LISTENING status to port.\n"); // - DBrooks
		if ( listen(sock_id, 1) != 0 ) 
			oops( "listen" );
	printf("Step 3 ended...\n\n"); // - DBrooks


	printNetworkInfo(&saddr,"SERVER"); //print host info using helper function - DBrooks
	

	/*
	* main loop: accept(), write(), close()
	*/
	printf("Server is now listening on socket/port...\n\n"); // - DBrooks
	while ( 1 )
	{
		//accept code edited by Brandon
		//client struct created
			struct sockaddr_in clientAddr;
			socklen_t slen = sizeof(clientAddr);
		// accept now pushes the client information into a new struct
			sock_fd = accept(sock_id, (struct sockaddr *)&clientAddr, &slen);
			printf("Wow! got a call!\n");
			printNetworkInfo(&clientAddr, "CLIENT");

			char ClientIPString[INET_ADDRSTRLEN];
			struct sockaddr_in* ClientAddrPointer = &clientAddr;
			inet_ntop(AF_INET, &(ClientAddrPointer->sin_addr), ClientIPString, sizeof(ClientIPString));
	
		//determine if Client tranmission is allowed: - DBrooks
			bool isAllowed = isAllowedToConnect(ClientIPString);
			//printf("Out of Allowed function.\n");
		
			//server replies to client:
			if ( sock_fd == -1 ) 							/* error getting calls  */
			{
				oops( "accept" )
			};       					

			sock_fp = fdopen(sock_fd,"w"); 					/* we'll write to the   */
				
			if ( sock_fp == NULL )							/* socket as a stream   */
			{       					
				oops( "fdopen" );       					/* unless we can't      */
			}

			//if client is allowed, send time
			if (isAllowed == true)
			{
				thetime = time(NULL);           				/* get time             */
														
	       		fprintf( sock_fp, "The time here is .." ); 		/* and convert to strng */
	       		fprintf( sock_fp, "%s", ctime(&thetime) ); 
				fclose( sock_fp );              				/* release connection   */
			}
			//if client is not allowed send rejection notice.
			else
			{
				printf("terminating connection with '%s', since they are not authorized...",ClientIPString);
				fprintf( sock_fp, "ACCESS DENIED, your IP is not permitted...\n"); 
				fclose( sock_fp );
			}
	}
};

//helper functions:

void printNetworkInfo(struct sockaddr_in *SockStruct, char *Type) // - DBrooks
{
	char IPAddrBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &SockStruct->sin_addr, IPAddrBuffer, sizeof(IPAddrBuffer)); // gets binary socket from struct and converts to IP format.
		
	uint16_t port = htons(SockStruct->sin_port);

	printf("%s's INFO %s:%d\n\n",Type,IPAddrBuffer,port);
};

bool isAllowedToConnect(char *ClientIP) // - DBrooks
{

	//printf("IP to check '%s'\n",ClientIP);
	printf("Checking file to see if client is allowed to connect fully...\n");

	bool result;
	FILE *FilePointer; 
	char Buffer[INET_ADDRSTRLEN];

	//printf("BufferSize = %i\n", INET_ADDRSTRLEN);

	if((FilePointer = fopen("AllowedList.txt","r")) == NULL)
	{
		printf("ERROR, could not open allow file...terminating\n");
		exit(-1);
	}

	while((fgets(Buffer, INET_ADDRSTRLEN, FilePointer) != NULL))
	{
		//puts(Buffer);
		//printf("FilePointer: %p", FilePointer);
		//printf("'%s' vs '%s' \n", Buffer, ClientIP);
		
		if (strstr(Buffer, ClientIP) != NULL)
		{
			printf("-> Client IP is allowed to connect :) ...\n");
			fclose(FilePointer);
			return true;
		}
	}

	printf("-> Client IP is NOT allowed to connect...\n");
	fclose(FilePointer);
	return false;
};