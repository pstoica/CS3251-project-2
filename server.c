/*///////////////////////////////////////////////////////////
*
* FILE:     server.c
* AUTHOR:   Patrick Stoica and Alex Saltiel
* PROJECT:  CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  GTmyMusic Server Code
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

#include "server.h"
#include "utilities.h"

#define MAXPENDING 5        /* Maximum outstanding connection requests */

list *file_list;
void *thread_main(void *args); /* Main program of a thread */
void handle_client(int clientSock);           /* TCP client handling function */

int main(int argc, char *argv[]) {
    int servSock;
    int clientSock;
    unsigned short port;
    pthread_t threadID;
    struct thread_args *threadArgs;

    file_list = create_list();

    /* Test for correct number of arguments */
    if (argc != 2) {
        fprintf(stderr,"Usage: %s <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]); /* First argument: local port */
    servSock = setup_server_socket(port);
    printf("GTmyMusic server started.\n");

    while (true) {
        clientSock = accept_connection(servSock);

        if ((threadArgs = (struct thread_args *) malloc(sizeof(struct thread_args))) == NULL) {
            die_with_error("pthread_create() failed");
        }

        threadArgs->clientSock = clientSock;

        if (pthread_create(&threadID, NULL, thread_main, (void *) threadArgs) != 0) {
            die_with_error("pthread_create() failed");
        }

        printf("with thread %ld\n", (long int) threadID);
    }
}

void *thread_main(void *threadArgs) {
    int clientSock;

    pthread_detach(pthread_self());

    clientSock = ((struct thread_args *) threadArgs)->clientSock;
    free(threadArgs);

    handle_client(clientSock);

    return (NULL);
}

void handle_client(int clientSock) {
    // FIXME: log signin
    list *client_list = create_list();
    list *diff_list = create_list();
    list *temp_list = create_list();
    printf("User signed in.\n");

    while (true) {
        char *request;
        char *message = get_request(clientSock);
        request = strtok(message, "\r\n");
        printf("Received Request: %s\n", request);

        // parse commands
        if (strcmp(request, "LIST") == 0) {
            empty_list(client_list, free_file);
            read_directory(client_list);
            // Goes through the list and sends the filename and checksum to client
            build_and_send_list(client_list, clientSock);
            
        } else if (strcmp(request, "DIFF") == 0 || strcmp(request, "PULL") == 0) {
            message = get_request(clientSock);

            empty_list(client_list, free_file);
            deserialize(client_list, message);

            empty_list(file_list, free_file);
            read_directory(file_list);

            empty_list(diff_list, free_file);
            traverse_diff(file_list, client_list, diff_list, file_comparator);

            build_and_send_list(diff_list, clientSock);
            
        } else if (strcmp(request, "LEAVE") == 0) {
            send_message("Bye!\r\n", clientSock);

            // FIXME: log signout
            printf("User signed out.\n");

            close(clientSock);
            break;
        } else if(strcmp(request, "sendfile") == 0) {
        	char *file_req = strtok(NULL, "\r\n");
        	printf("%s requested\n", file_req);
        	send_file(file_req, clientSock);
        	printf("%s file sent\n", file_req);
        	
        } else {
            send_message("Invalid request.\n", clientSock);
        	// send back help or invalid command notification msg
        }
    }
}
