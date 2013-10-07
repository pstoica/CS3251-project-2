#include <stdbool.h>
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
#include "utilities.h"

char *get_request(int clientSock) {
    char buffer[BUFFER_SIZE];
    int received_size = 0;
    int message_size = 0;
    int capacity = BUFFER_SIZE;
    int i;

    char *message = calloc(1, sizeof(char) * BUFFER_SIZE);
    assert(message);

    do {
        //printf("Receiving...\n");
        received_size = recv(clientSock, buffer, BUFFER_SIZE, 0);
        //printf("Message length: %d\n", received_size);

        if (received_size < 0) {
            free(message);
            return 0;
        } else if (received_size > 0) {
            for (i = 0; i < received_size; i++) {
                message[message_size + i] = buffer[i];
            }

            message_size += received_size;
            capacity += BUFFER_SIZE;
            message = realloc(message, capacity);

            memset(buffer, 0, BUFFER_SIZE);
        }

        //printf("%s\n", message);
    } while (!is_valid(message));

    return message;
}

void send_file(char *filename, int clnt_sock){
	struct stat stbuf;
	FILE *fp;
	char buff[BUFFER_SIZE];
	unsigned long int filesize = 0;
	unsigned long int sentbytes = 0;
	
	memset(buff, 0, BUFFER_SIZE);
	
	if (stat(filename, &stbuf) == -1) {
		die_with_error("stat() failed");
	}
    filesize = (unsigned long int) stbuf.st_size;
    
    fp = fopen(filename,"r");
    if(fp == 0 || fp == NULL) die_with_error("fopen() failed");
    
    while(sentbytes < filesize)
	{
		uint32_t read = fread(buff,1,BUFFER_SIZE,fp);
		uint32_t written = send(clnt_sock, buff, read,0);
		sentbytes += written;
		memset(buff, 0, BUFFER_SIZE);
	}
	
	fclose(fp);
}

void recv_file(filenode *file, int sock){
	FILE *fp;
	char buff[BUFFER_SIZE];
	unsigned long int recvbytes = 0;
	unsigned long int byteswritten = 0;
	
	memset(buff, 0, BUFFER_SIZE);
	
	fp = fopen(file->name, "w");
	if(fp == 0 || fp == NULL) die_with_error("fopen() failed");
	
	uint32_t bytes = 0;
	do {
		bytes = recv(sock, buff, BUFFER_SIZE, 0);
		if(bytes > 0){
			recvbytes += bytes;
			uint32_t written = fwrite(buff,1,bytes,fp);
			byteswritten += written;
			memset(buff, 0, BUFFER_SIZE);
		}
	} while(recvbytes < file->size && bytes > 0);
	
	fclose(fp);
}

int is_valid(char *message) {
    int length = strlen(message);

    // matches \r\n
    return (message[length - 2] == 13 && message[length - 1] == 10);
}

int setup_server_socket(unsigned short port) {
    int servSock;
    struct sockaddr_in servAddr;      /* Local address */

    if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        die_with_error("could not create socket");
    }

    /* Construct local address structure*/
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    /* Bind to local address structure */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        die_with_error("bind() failed");
    }

    /* Listen for incoming connections */
    if (listen(servSock, MAX_CONNECTIONS) < 0) {
        die_with_error("listen() failed");
    }

    return servSock;
}

int send_message(char *message, int sock) {
    int bytes_sent = 0;
    int message_length = strlen(message);
    
    bytes_sent = send(sock, message, message_length, 0);

    if (bytes_sent < 0) {
        die_with_error("send() failed");
    } else if (bytes_sent != message_length) {
        die_with_error("send() - sent unexpected number of bytes\n");
    }
}

int accept_connection(int servSock) {
    struct sockaddr_storage clientAddr; // Client address
    socklen_t clientAddrLen = sizeof(clientAddr);

    /* wait for a client to connect */
    int clientSock = accept(servSock, (struct sockaddr *) &clientAddr, &clientAddrLen);
    if (clientSock < 0) {
        die_with_error("accept() failed");
    }

    return clientSock;
}

void die_with_error(char *message) {
    perror(message);
    exit(1);
}

// All List functionality not working, but reads through and gets all regular files in current dir
void read_directory(list *file_list){
	struct stat stbuf;
	// Create directory handle
	DIR *d;
	struct dirent *dir;
	
	// File Handle
	FILE *fp;

	// Open directory
	d = opendir(".");

    //printf("opendir\n");
	if(d == NULL)
		return;
	
	if(d) {
        //printf("found d\n");
		while((dir = readdir(d)) != NULL) {
			char *file_name = dir->d_name;
			char *ext = strrchr(file_name, '.');
			if(ext == NULL) continue;
			if(strcmp(ext, ".mp3")) continue;
			ext = NULL;
			
			// Only adds 'regular' files to linked list
			if(dir->d_type == DT_REG){

                filenode *file = malloc(sizeof(filenode));
                file->name = file_name;
                
                if (stat(file->name, &stbuf) == -1) {
					die_with_error("stat() failed");
				}
                file->size = (unsigned long int) stbuf.st_size;              
				
				// Checksum works on mp3 file and returns an unsigned int
				if((fp = fopen(file_name, "rb")) != NULL){
                    file->checksum = checksum(fp);
                    fclose(fp);
				}
				
				//pushes file name into the list
				push_back(file_list, file);
                //printf("added file %s\n%s\n%lu\n", file_name, file->checksum, file->size);
			} 
		}
		// Close directory
		closedir(d);
	}
}

static char *checksum(FILE *inFile) {
    MD5_CTX mdContext;
    unsigned char digest[16];
    char *checksum = (char*)malloc(33);
    int n;
    int bytes;
    unsigned char data[1024];

    MD5_Init(&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0) {
        MD5_Update(&mdContext, data, bytes);
    }

    MD5_Final(digest, &mdContext);

    for (n = 0; n < 16; ++n) {
        snprintf(&(checksum[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return checksum;
}

void deserialize(list *file_list, char *message) {
    int count = 0;
    char *file_name;
    char *checksum;
    char *token;
    unsigned long size;

    token = strtok(message, "\r\n");
    count++;

    while (token != NULL) {   
        //printf("found token: %s\n", token);

        if (count == 1) {
            file_name = token;
        } else if (count == 2) {
            checksum = token;
        } else if(count == 3){
        	size = strtoul(token, NULL, 0);
        }

        if (count == 3) {
            //printf("found file node\n");
            filenode *file = malloc(sizeof(filenode));
            file->name = file_name;
            file->checksum = checksum;
            file->size = (unsigned long int)size;

            push_back(file_list, file);
            count = 0;
        }

        token = strtok(NULL, "\r\n");
        count++;
    }
}

void build_and_send_list(list *file_list, int clnt_sock){
	filenode *current = front(file_list);
    char *buffer;
    int buffer_size = 0;
    int i;

    if (is_empty(file_list)) {
        send_message("\r\n", clnt_sock);
    }

	while(!is_empty(file_list)) {
        remove_front(file_list, free_file);

        if (is_empty(file_list)) {
            // end, add return carriage
            buffer_size = asprintf(&buffer, "%s\n%s\n%lu\r\n", current->name, current->checksum, current->size);
        } else {
            buffer_size = asprintf(&buffer, "%s\n%s\n%lu\n", current->name, current->checksum, current->size);
        }

        send_message(buffer, clnt_sock);
        free(buffer);

        current = front(file_list);
	}
}



int file_comparator(const void *data1, const void *data2) {
    int result;
    filenode *file1 = (filenode *) data1;
    filenode *file2 = (filenode *) data2;

    result = ((strcmp(file1->name, file2->name) == 0) &&
              (strcmp(file1->checksum, file2->checksum) == 0));

    return result;
}

void print_files(void *data) {
    filenode *file = (filenode *) data;
    printf("%s\n", file->name);
    printf("%s\n", file->checksum);
}

void print_filenames(void *data) {
    filenode *file = (filenode *) data;
    printf("%s\n", file->name);
}

void free_file(void *data) { }

char *timestamp() {
	char *ts = (char *)malloc(sizeof(char)*50);
	time_t ltime;
	ltime = time(NULL);
	asctime_r(localtime(&ltime), ts);
	
	char *tmp = ts;
	
	while (*tmp) {
        if (*tmp == '\n') {
            *tmp='\0';
        }
        tmp++;
    }
	
	return ts;
}
