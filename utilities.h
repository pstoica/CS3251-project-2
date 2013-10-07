#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <time.h>
#include "list.h"

#pragma once

#define RECEIVE_BUFFER_SIZE 512      /* The receive buffer size */
#define SEND_BUFFER_SIZE 512      /* The send buffer size */
#define MAXPENDING 5
#define current_ts() (unsigned int)time(NULL)

struct thread_args {
    int clientSock;
};

/* stores filename and checksum */
typedef struct lfilenode {
  char *name;
  char *checksum;
  unsigned long int size;
} filenode;

char *concatenate(char *message, char *string);
char *get_request(int sock);
int is_valid(char *message);
void die_with_error(char *errorMessage);        			/* Error handler */
int setup_server_socket(unsigned short port);  				/* Create TCP server socket */
int accept_connection(int servSock);          				/* Accept TCP connection request */
void read_directory(list *file_list);						/* Reads the file names in the given directory */
void print_files(void *data);                   			/* Prints file data */
void print_filenames(void *data);                           /* Prints only file names */
void free_file(void *data);
static char *checksum(FILE *inFile);            			/* Calculates md5 checksum for given filepointer */
void build_and_send_list(list *file_list, int clnt_sock);	
void send_file(char *filename, int clnt_sock);
void recv_file(filenode *file, int sock);
void deserialize(list *file_list, char *message);
int file_comparator(const void *data1, const void *data2);
char *timestamp();


