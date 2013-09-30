/*///////////////////////////////////////////////////////////
*
* FILE:		client.c
* AUTHOR:	Alex Saltiel and Patrick Stoica
* PROJECT:	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:	GTmyMusic Client Code
*
*////////////////////////////////////////////////////////////

/* Included libraries */
#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
#include <stdlib.h>         /* supports all sorts of functionality */
#include <unistd.h>         /* for close() */
#include <string.h>         /* support any string ops */
#include <stdbool.h>
#include <ctype.h>			/* for toupper() for command case formatting */
#include "utilities.h"

#define CMDLEN 5				/* for the cmd and the \n */
#define INPUT_BUFFER_MAX 80		/* max length of input buffer */


/*
 *	Thoughts
 *		Consider taking in dir to music home when started
 */
int main(int argc, char *argv[]){
	int clientSock;					/* socket discriptor */
	struct sockaddr_in serv_addr; 	/* the server address */
	int i;
	
	char *input = (char *)malloc(INPUT_BUFFER_MAX);	
	char sndBuf[SEND_BUFFER_SIZE];
	char rcvBuf[RECEIVE_BUFFER_SIZE];
	
	size_t msgLength;
	ssize_t numBytes;
	unsigned int totalBytesRcvd;
	
	/* zero out all buffers */
	memset(&sndBuf, 0, SEND_BUFFER_SIZE);
	memset(&rcvBuf, 0, RECEIVE_BUFFER_SIZE);
	memset(input, 0, INPUT_BUFFER_MAX);
	
	/* Create a new TCP socket */
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
		die_with_error("socket() failed\n");
		
	/* Construct the server address structure */
	memset(&serv_addr, 0, sizeof(serv_addr));	// zero out the structure
	serv_addr.sin_family = AF_INET;				// IPv4 address family
	serv_addr.sin_addr.s_addr = inet_addr("130.207.114.21");
	serv_addr.sin_port = htons(4356);
	
	/* Connect to the server */
	if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		die_with_error("connect() failed");	
	
	while(true){
		//print welcome message (get from server probably)
		printf("GTmyMusic >>> ");
		
		/* wait for input from the client user */
		scanf("%s", input);
		//for(i = 0; input[i] != '\0' && i < strlen(input); i++) input[i] = toupper(input[i]);
		
		/* if any command besides DIFF or LEAVE, just send the cmd */
		if(strcmp(input, "DIFF")){
			strcpy(sndBuf, input);
			msgLength = strlen(sndBuf);
			numBytes = send(sock, sndBuf, msgLength, 0);
			if(numBytes < 0)
				die_with_error("send() failed");
			else if(numBytes != msgLength)
				die_with_error("send() - sent unexpected # of bytes\n");
				
			if(strcmp(input, "LEAVE") == 0) break;
			
		// otherwise, get dir info and send with DIFF command
		} else {
			// get client directory crawl list
			// send the cmd and the file list \n seperated
		}
		
		/* Receive response until entire response received */
		totalBytesRcvd = 0;
		printf("Response from server: ");
		 while(numBytes > 0){
			numBytes = recv(sock, rcvBuf, RECEIVE_BUFFER_SIZE, 0);
			if(numBytes < 0)
				die_with_error("recv() failed");
			printf("%s", rcvBuf);
			fflush(stdout);		
			totalBytesRcvd+= numBytes;
		}
		printf("\n");
	}
	
	close(sock);
	
	return 0;
}










