#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include "list.h"

#pragma once

#define RECEIVE_BUFFER_SIZE 512      /* The receive buffer size */
#define SEND_BUFFER_SIZE 512      /* The send buffer size */
#define MAXPENDING 5

char *get_request(int sock);
int is_valid(char *message);
void die_with_error(char *errorMessage);        			/* Error handler */
int setup_server_socket(unsigned short port);  				/* Create TCP server socket */
int accept_connection(int servSock);          				/* Accept TCP connection request */
void read_directory(list *file_list);						/* Reads the file names in the given directory */
void print_files(void *data);                   			/* Prints file data */
static char *checksum(FILE *inFile);            			/* Calculates md5 checksum for given filepointer */
void build_and_send_list(list *file_list, int clnt_sock);	/* */

struct thread_args {
    int clientSock;
};

/* stores filename and checksum */
typedef struct lfilenode {
  char *name;
  char *checksum;
} filenode;
