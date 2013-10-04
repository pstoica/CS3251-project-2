/*///////////////////////////////////////////////////////////
*
* FILE:		client.c
* AUTHOR:	Alex Saltiel and Patrick Stoica
* PROJECT:	CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:	GTmyMusic Client Code
*
*////////////////////////////////////////////////////////////

/* Included libraries */

#include "client.h"
#include "utilities.h"

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

    if (argc != 3) {
        fprintf(stderr,"Usage: %s <SERVER HOST> <SERVER PORT>\n", argv[0]);
        exit(1);
    }
	
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
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	
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
			//run LIST logic and send to server
		} else if (strcmp(input, "LIST") == 0) {
			send_message("LIST\r\n", sock);
			//receive data until some EOD flag received
		} else if (strcmp(input, "PULL") == 0) {
			send_message("PULL\r\n", sock);
		} else if (strcmp(input, "LEAVE") == 0) {
			send_message("LEAVE\r\n", sock);
			exit(0);
		}
		
		if (strcmp(input, "LIST")){
		    char *message = get_request(sock);
		    printf("Received: %s", message);
		} else {
			char *message = get_request(sock);
			printf("%s\n", message);
			message = get_request(sock);
			printf("%s\n", message);
		}
	}
	
	close(sock);
	
	return 0;
}










