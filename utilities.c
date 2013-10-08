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

bool send_data(int sock, void *data, int size) {
    int sent = 0;
    int remaining = size;
    int result;

    if (data == NULL || size == 0) return true;

    while (remaining > 0) {
        result = send(sock, (unsigned char *) data + sent, remaining, 0);

        if (result < 0) die_with_error("write() failed");

        sent += result;
        remaining -= result;
    }

    return true;
}

bool get_request_header(int sock, request_header *header, int size) {
    int received = 0;
    int remaining = size;
    int result;

    while (remaining > 0) {
        result = recv(sock, (unsigned char *) header + received, remaining, 0);

        if (result < 0) die_with_error("read() in get_request failed");

        received += result;
        remaining -= result;
    }

    return true;
}

bool get_response_header(int sock, response_header *header, int size) {
    int received = 0;
    int remaining = size;
    int result;

    while (remaining > 0) {
        result = recv(sock, (unsigned char *) header + received, remaining, 0);

        if (result < 0) die_with_error("read() in get_response failed");

        received += result;
        remaining -= result;
    }

    return true;
}

unsigned char *get_data(int sock, int size) {
    int received = 0;
    int remaining = size;
    int result;
    char *buffer = malloc(size);

    memset(buffer, 0, size);

    while (remaining > 0) {
        result = recv(sock, (unsigned char *) buffer + received, remaining, 0);

        if (result < 0) die_with_error("read() failed");

        received += result;
        remaining -= result;
    }

    return buffer;
}

bool send_file(int sock, response *res, char *filename) {
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

    res->header.size = filesize;
    send_data(sock, &(res->header), sizeof(res->header));
    
    while(sentbytes < filesize)
    {
        uint32_t read = fread(buff,1,BUFFER_SIZE,fp);
        uint32_t written = send(sock, buff, read,0);
        sentbytes += written;
        memset(buff, 0, BUFFER_SIZE);
    }
    
    fclose(fp);
}

void recv_file(int sock, file_entry *file, int size) {
    FILE *fp;
    int received = 0;
    int remaining = size;
    int result;
    char *buffer = malloc(size);
    
    memset(buffer, 0, size);
    
    fp = fopen(file->name, "w");
    if(fp == 0 || fp == NULL) die_with_error("fopen() failed");

    while (remaining > 0) {
        result = recv(sock, (unsigned char *) buffer + received, remaining, 0);

        if (result < 0) die_with_error("read() failed");

        received += result;
        remaining -= result;
    }

    uint32_t written = fwrite(buffer, 1, received, fp);
    
    fclose(fp);
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

                file_entry *file = malloc(sizeof(file_entry));
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

void deserialize_list(list *file_list, char *message) {
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
            file_entry *file = malloc(sizeof(file_entry));
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

int file_comparator(const void *data1, const void *data2) {
    int result;
    file_entry *file1 = (file_entry *) data1;
    file_entry *file2 = (file_entry *) data2;

    result = ((strcmp(file1->name, file2->name) == 0) &&
              (strcmp(file1->checksum, file2->checksum) == 0));

    return result;
}

void print_files(void *data) {
    file_entry *file = (file_entry *) data;
    printf("%s\n", file->name);
    printf("%s\n", file->checksum);
}

char *file_to_string(void *data) {
    char *buffer;
    file_entry *file = (file_entry *) data;

    asprintf(&buffer, "%s\n%s\n%lu\n", file->name, file->checksum, file->size);
    return buffer;
}

void print_filenames(void *data) {
    file_entry *file = (file_entry *) data;
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
