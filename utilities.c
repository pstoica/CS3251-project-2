#include <stdbool.h>
#include "utilities.h"

char *get_request(int clientSock) {
    char buffer[RECEIVE_BUFFER_SIZE];
    int received_size = 0;
    int message_size = 0;
    int capacity = RECEIVE_BUFFER_SIZE;

    char *message = calloc(1, sizeof(char) * RECEIVE_BUFFER_SIZE);
    assert(message);

    do {
        printf("Receiving...\n");
        received_size = recv(clientSock, buffer, RECEIVE_BUFFER_SIZE, 0);
        printf("Message length: %d\n", received_size);

        if (received_size < 0) {
            free(message);
            return 0;
        } else if (received_size > 0) {
            int i = 0;

            for (i = 0; i < received_size; i++) {
                message[message_size + i] = buffer[i];

                /* if our message size is about to go over our capacity,
                   increase the string size and reallocate */
                if (message_size >= capacity) {
                    printf("regrowing message\n");
                    capacity += RECEIVE_BUFFER_SIZE;
                    message = realloc(message, capacity);
                }
            }

            message_size += received_size;

            memset(buffer, 0, RECEIVE_BUFFER_SIZE);
        }
    } while (!is_valid(message));

    return message;
}

int is_valid(char *message) {
    int length = strlen(message);

    if (length < 2) return false;
    return ((message[length - 2] == '\r') && (message[length - 1] == '\n'));
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
    if (listen(servSock, MAXPENDING) < 0) {
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
	// Create directory handle
	DIR *d;
	struct dirent *dir;
	
	// File Handle
	FILE *fp;

	// Open directory
	d = opendir(".");

    printf("opendir\n");
	if(d == NULL)
		return;
	
	if(d) {
        printf("found d\n");
		while((dir = readdir(d)) != NULL) {
            printf("file found\n");
			if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
				continue;
			
			// Only adds 'regular' files to linked list
			if(dir->d_type == DT_REG){
				char *file_name = dir->d_name;

                filenode *file = malloc(sizeof(filenode));
                file->name = file_name;
				
				// Checksum works on mp3 file and returns an unsigned int
				if((fp = fopen(file_name, "rb")) != NULL){
                    file->checksum = checksum(fp);

                    fclose(fp);
				}
				
				//pushes file name into the list
				push_back(file_list, file);
                printf("added file %s\n", file_name);
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

void print_files(void *data) {
    filenode *file = (filenode *) data;
    printf("%s\n%s\n", file->name, file->checksum);
}