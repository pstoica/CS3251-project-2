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
#define INPUT_BUFFER_MAX 32		/* max length of input buffer */

list *local_list;
list *server_list;

int parse_command(char *command) {
	if (command == NULL) {
		return -1;
	}

	size_t length = strlen(command) - 1;
	if (command[length] == '\n') command[length] = '\0';

	if (strcmp(command, "LIST") == 0) {
		return LIST;
	} else if (strcmp(command, "DIFF") == 0) {
		return DIFF;
	} else if (strcmp(command, "PULL") == 0) {
		return PULL;
	} else if (strcmp(command, "LEAVE") == 0) {
		return LEAVE;
	} else if (strcmp(command, "FETCH") == 0) {
		return FETCH;
	} else  {
		return -1;
	}
}

void perform_list(int sock) {
	request req;
	response res;

	req.header.type = LIST;
	req.header.size = 0;
	req.data = NULL;

	send_data(sock, &(req.header), sizeof(req.header));
	get_response_header(sock, &(res.header), sizeof(res.header));
	res.data = get_data(sock, res.header.size);

	empty_list(server_list, free_file);
	deserialize_list(server_list, res.data);

	if (server_list->size == 0) {
		printf("No files found.\n");
	} else {
		traverse(server_list, print_filenames);
	}
}

void perform_diff(int sock) {
	char *list;
	request req;
	response res;

	empty_list(local_list, free_file);
	read_directory(local_list);
	list = traverse_to_string(local_list, file_to_string);

	req.header.type = DIFF;
	req.header.size = sizeof(char) * (strlen(list) + 1);
	req.data = list;

	send_data(sock, &(req.header), sizeof(req.header));
	send_data(sock, req.data, req.header.size);
	get_response_header(sock, &(res.header), sizeof(res.header));
	res.data = get_data(sock, res.header.size);

	empty_list(server_list, free_file);
    deserialize_list(server_list, res.data);

	if (server_list->size == 0) {
		printf("No files found.\n");
	} else {
		traverse(server_list, print_filenames);
	}
}

void perform_pull(int sock) {

}

void perform_fetch(int sock) { }

void perform_leave(int sock) {
	request req;

	req.header.type = LEAVE;
	req.header.size = 0;
	req.data = NULL;

	send_data(sock, &(req.header), sizeof(req.header));
	exit(0);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in serv_addr;
	int i;

	/*
	size_t msgLength;
	ssize_t numBytes;
	unsigned int totalBytesRcvd;
	*/
	
	char *input = (char *) malloc(INPUT_BUFFER_MAX);
	char sndBuf[BUFFER_SIZE];
	char rcvBuf[BUFFER_SIZE];

	/* zero out all buffers */
	memset(&sndBuf, 0, BUFFER_SIZE);
	memset(&rcvBuf, 0, BUFFER_SIZE);
	memset(input, 0, INPUT_BUFFER_MAX);

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <SERVER HOST> <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    /* initialize linked lists */
    local_list = create_list();
    server_list = create_list();
	
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
		fgets(input, INPUT_BUFFER_MAX, stdin);

		switch(parse_command(input)) {
			case LIST:
				perform_list(sock);
				break;
			case DIFF:
				perform_diff(sock);
				break;
			case PULL:
				perform_pull(sock);
				break;
			case LEAVE:
				perform_leave(sock);
				break;
		}

		/*if (strcmp(input, "DIFF") == 0) {
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
		}*/

		/*char *message = get_request(sock);
		
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
		    	
		    	file_entry *current = front(server_list);
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

		free(message);*/
	}
	
	close(sock);
	
	return 0;
}
