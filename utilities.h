#include "list.h"

#pragma once

#define BUFFER_SIZE 512      /* The receive buffer size */
#define MAX_CONNECTIONS 5

struct thread_args {
    int clientSock;
};

/* stores filename and checksum */
typedef struct lfilenode {
  char *name;
  char *checksum;
  unsigned long int size;
} filenode;

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
