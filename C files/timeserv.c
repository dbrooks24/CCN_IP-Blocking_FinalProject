/* timeserv.c - a socket-based time of day server
 */

#include  <stdio.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netdb.h>
#include  <time.h>
#include  <strings.h>

#define _OPEN_SYS_SOCK_IPV6
#include <arpa/inet.h>



#define   PORTNUM  13000   /* our time service phone number */
#define   HOSTLEN  256
#define   oops(msg)      { perror(msg) ; exit(1) ; }

void printIP(struct sockaddr_in *SockStruct); // helper function to print server address and port - Added by DBrooks

int main(int ac, char *av[])
{
	struct  sockaddr_in   saddr;   /* build our address here */	
	struct	hostent		*hp;   /* this is part of our    */
	char	hostname[HOSTLEN];     /* address 	         */
	int	sock_id,sock_fd;       /* line id, file desc     */
	FILE	*sock_fp;              /* use socket as stream   */
	char    *ctime();              /* convert secs to string */
	time_t  thetime;               /* the time we report     */

	/*
	* Step 1: ask kernel for a socket
	*/
	printf("Step 1 started...\n"); // - added by DBrooks
		sock_id = socket( PF_INET, SOCK_STREAM, 0 );    /* get a socket */
		if ( sock_id == -1 ) 
			oops( "socket" );
		printf("\tsocket access was given by kernel...\n");
	printf("Step 1 ended...\n\n"); // - added by DBrooks


	/*
	* Step 2: bind address to socket.  Address is host,port
	*/
	printf("Step 2 started...\n"); // - added by DBrooks
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
	printf("Step 3 started...\n"); // - added by DBrooks
		if ( listen(sock_id, 1) != 0 ) 
			oops( "listen" );
	printf("Step 3 ended...\n"); // - added by DBrooks
	printf("\tServer is now listening on socket...\n"); // - added by DBrooks

	printIP(&saddr); //print host info using helper function - added by DBrooks
	

	/*
	* main loop: accept(), write(), close()
	*/
	printf("\tNow entering accept loop...\n"); //added by DBrooks
	while ( 1 )
	{
	    sock_fd = accept(sock_id, NULL, NULL); /* wait for call */
		printf("Wow! got a call!\n");
		if ( sock_fd == -1 )
			oops( "accept" );       /* error getting calls  */

		sock_fp = fdopen(sock_fd,"w");  /* we'll write to the   */
	       if ( sock_fp == NULL )          /* socket as a stream   */
			oops( "fdopen" );       /* unless we can't      */

		thetime = time(NULL);           /* get time             */
			/* and convert to strng */
	       fprintf( sock_fp, "The time here is .." );
	       fprintf( sock_fp, "%s", ctime(&thetime) ); 
		fclose( sock_fp );              /* release connection   */
	}
};


void printIP(struct sockaddr_in *SockStruct) //def of print function - Added by DBrooks
{
	char IPAddrBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &SockStruct->sin_addr, IPAddrBuffer, sizeof(IPAddrBuffer)); // gets binary socket from struct and converts to IP format.
		
	uint16_t port = htons(SockStruct->sin_port);

	printf("SERVER INFO: %s:%d\n\n",IPAddrBuffer,port);
};