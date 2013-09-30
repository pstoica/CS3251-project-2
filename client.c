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
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(4356);
	
	/* Connect to the server */
	if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		die_with_error("connect() failed");	
	}

	printf("Welcome to GTmyMusic.\n");

	while (true) {

		scanf("%s", input);
		printf("Sending %s request...\n", input);

		if (strcmp(input, "DIFF") == 0) {
			send_message("DIFF\r\n", sock);
		} else if (strcmp(input, "LIST") == 0) {
			send_message("LIST\r\n", sock);
		} else if (strcmp(input, "PULL") == 0) {
			send_message("PULL\r\n", sock);
		} else if (strcmp(input, "LEAVE") == 0) {
			send_message("LEAVE\r\n", sock);
			exit(0);
		}
		
        char *message = get_request(sock);
        printf("Received: %s", message);
	}
	
	close(sock);
	
	return 0;
}










