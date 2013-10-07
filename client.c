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

list *local_list;
list *server_list;
int i;

/*
 *	Thoughts
 *		Consider taking in dir to music home when started
 */
int main(int argc, char *argv[]){
	struct sockaddr_in serv_addr; 	/* the server address */
	int i;
	
	char *input = (char *)malloc(INPUT_BUFFER_MAX);	
	char sndBuf[BUFFER_SIZE];
	char rcvBuf[BUFFER_SIZE];
	
	size_t msgLength;
	ssize_t numBytes;
	unsigned int totalBytesRcvd;

    if (argc != 3) {
        fprintf(stderr,"Usage: %s <SERVER HOST> <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    /* initialize linked lists */
    local_list = create_list();
    server_list = create_list();
	
	/* zero out all buffers */
	memset(&sndBuf, 0, BUFFER_SIZE);
	memset(&rcvBuf, 0, BUFFER_SIZE);
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

		if (strcmp(input, "DIFF") == 0) {
			send_message("DIFF\r\n", sock);

			empty_list(local_list, free_file);
			read_directory(local_list);
			build_and_send_list(local_list, sock);
		} else if (strcmp(input, "LIST") == 0) {
			send_message("LIST\r\n", sock);
			
		} else if (strcmp(input, "PULL") == 0) {
			send_message("PULL\r\n", sock);
			
			empty_list(local_list, free_file);
			read_directory(local_list);
			build_and_send_list(local_list, sock);
		} else if (strcmp(input, "LEAVE") == 0) {
			send_message("LEAVE\r\n", sock);
			exit(0);
		}

		char *message = get_request(sock);
		
		if (strcmp(input, "LIST") == 0 || strcmp(input, "DIFF") == 0){
		    empty_list(server_list, free_file);
		    deserialize(server_list, message);

		    if (server_list->size == 0) {
		    	printf("No files found.\n");
		    } else {
				traverse(server_list, print_filenames);
		    }
		} else if(strcmp(input, "PULL") == 0) {
			empty_list(server_list, free_file);
		    deserialize(server_list, message);

		    if (server_list->size == 0) {
		    	printf("No files pulled.\n");
		    } else {
		    	traverse(server_list, print_filenames);
		    	
		    	filenode *current = front(server_list);
		    	char *buffer;
    			int buffer_size = 0;
    			FILE *file;
		    	while(!is_empty(server_list)){
		    		remove_front(server_list, free_file);
		    		//remove file if exists
		    		if((file = fopen(current->name, "r")) != NULL){
		    			fclose(file);
		    			remove(current->name);
		    		}
		    		buffer_size = asprintf(&buffer, "sendfile\r\n%s\r\n", current->name);
		    		//send recv ready msg
		    		send_message(buffer, sock);
		    		free(buffer);
		    		//recv and save as new file
		    		printf("receiving %s\n", current->name);
		    		recv_file(current, sock);
		    		printf("%s recieved\n", current->name);
		    		current = front(server_list);
		    	}
		    }
		} else {
			printf("%s\n", message);
		}

		free(message);
	}
	
	close(sock);
	
	return 0;
}










