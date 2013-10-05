#include <stdbool.h>
#include "utilities.h"

char *get_request(int clientSock) {
    char buffer[RECEIVE_BUFFER_SIZE];
    int received_size = 0;
    int message_size = 0;
    int capacity = RECEIVE_BUFFER_SIZE;
    int i;

    char *message = calloc(1, sizeof(char) * RECEIVE_BUFFER_SIZE);
    assert(message);

    do {
        //printf("Receiving...\n");
        received_size = recv(clientSock, buffer, RECEIVE_BUFFER_SIZE, 0);
        //printf("Message length: %d\n", received_size);

        if (received_size < 0) {
            free(message);
            return 0;
        } else if (received_size > 0) {
            for (i = 0; i < received_size; i++) {
                message[message_size + i] = buffer[i];
            }

            message_size += received_size;
            capacity += RECEIVE_BUFFER_SIZE;
            message = realloc(message, capacity);

            memset(buffer, 0, RECEIVE_BUFFER_SIZE);
        }

        //printf("%s\n", message);
    } while (!is_valid(message));

    return message;
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

    //printf("opendir\n");
	if(d == NULL)
		return;
	
	if(d) {
        //printf("found d\n");
		while((dir = readdir(d)) != NULL) {
            //printf("file found\n");
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
                //printf("added file %s\n", file_name);
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

    token = strtok(message, "\r\n");
    count++;

    while (token != NULL) {   
        //printf("found token: %s\n", token);

        if (count == 1) {
            file_name = token;
        } else if (count == 2) {
            checksum = token;
        }

        if (count == 2) {
            //printf("found file node\n");
            filenode *file = malloc(sizeof(filenode));
            file->name = file_name;
            file->checksum = checksum;

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

	while(!is_empty(file_list)) {
        remove_front(file_list, free_file);

        if (is_empty(file_list)) {
            // end, add return carriage
            //printf("final line\n");
            buffer_size = asprintf(&buffer, "%s\n%s\r\n", current->name, current->checksum);
        } else {
            buffer_size = asprintf(&buffer, "%s\n%s\n", current->name, current->checksum);
        }

        send_message(buffer, clnt_sock);
        free(buffer);

        current = front(file_list);
	}
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