#pragma once

#include "list.h"

#define BUFFER_SIZE 512
#define MAX_CONNECTIONS 5

#define LIST  0
#define DIFF  1
#define PULL  2
#define FETCH 3
#define LEAVE 4

struct thread_args {
    int sock;
};

/* Represents file in directory */
typedef struct lfile_entry {
	char *name;
	char *checksum;
	unsigned long int size;
} file_entry;

/* Headers with predictable sizes */
typedef struct lrequest_header {
	unsigned int type;
	unsigned int size;
} request_header;

typedef struct lresponse_header {
	unsigned int status;
	unsigned int size;
} response_header;

/* Header + body, where header must be received first
   to retrieve data of some known size */
typedef struct lrequest {
	request_header header;
	void *data;
} request;

typedef struct lresponse {
	response_header header;
	void *data;
} response;

bool get_request_header(int sock, request_header *header, int size);
bool get_response_header(int sock, response_header *header, int size);

unsigned char *get_data(int sock, int size);
bool send_data(int sock, void *data, int size);

//char *get_request(int sock);
int is_valid(char *message);

void die_with_error(char *errorMessage);        			/* Error handler */
int setup_server_socket(unsigned short port);  				/* Create TCP server socket */
int accept_connection(int sock);          				    /* Accept TCP connection request */

void read_directory(list *file_list);						/* Reads the file names in the given directory */
void print_files(void *data);                   			/* Prints file data */
void print_filenames(void *data);                           /* Prints only file names */
void free_file(void *data);
char *file_to_string(void *data);
static char *checksum(FILE *inFile);            			/* Calculates md5 checksum for given filepointer */
char *build_list(list *file_list);
bool send_file(int sock, response *res, char *filename);
void recv_file(int sock, file_entry *file, int size);
void deserialize_list(list *file_list, char *message);
int file_comparator(const void *data1, const void *data2);
char *timestamp();
