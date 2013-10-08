/*///////////////////////////////////////////////////////////
*
* FILE:     server.c
* AUTHOR:   Patrick Stoica and Alex Saltiel
* PROJECT:  CS 3251 Project 2 - Professor Traynor
* DESCRIPTION:  GTmyMusic Server Code
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), connect(), send(), and recv() */
#include <sys/select.h>     /* for select() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
#include <stdlib.h>         /* supports all sorts of functionality */
#include <unistd.h>         /* for close() */
#include <string.h>         /* support any string ops */
#include <stdbool.h>
#include <pthread.h>
#include "list.h"
#include "utilities.h"

#define MAX_CONNECTIONS 5        /* Maximum outstanding connection requests */

list *file_list;
static int users = 0;

void *thread_main(void *args);   /* Main program of a thread */

void perform_list(int sock);
void perform_diff(int sock);
void perform_pull(int sock);
void perform_fetch(int sock);
void perform_leave(int sock);

void log_action(unsigned int user, char *message);
void get_client_ip(int clientSock, char *buf);

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

    /* local port */
    port = atoi(argv[1]);
    servSock = setup_server_socket(port);

    printf("GTmyMusic server started.\n");

    while (true) {
        clientSock = accept_connection(servSock);

        if ((threadArgs = (struct thread_args *) malloc(sizeof(struct thread_args))) == NULL) {
            die_with_error("pthread_create() failed");
        }

        threadArgs->sock = clientSock;

        if (pthread_create(&threadID, NULL, thread_main, (void *) threadArgs) != 0) {
            die_with_error("pthread_create() failed");
        }
    }
}

void perform_list(int sock) {
    printf("PERFORM LIST\n");
}

void perform_diff(int sock) { }
void perform_pull(int sock) { }
void perform_fetch(int sock) { }

void perform_leave(int sock) {
    close(sock);
}

void *thread_main(void *threadArgs) {
    bool running = true;
    int clientSock = ((struct thread_args *) threadArgs)->sock;

    pthread_detach(pthread_self());
    free(threadArgs);

    list *client_list = create_list();
    list *diff_list = create_list();

    int user = ++users;
    char clnt_ip[INET_ADDRSTRLEN+5]; 
    get_client_ip(clientSock, clnt_ip);
    char *msg;

    asprintf(&msg, "User signed in (%s)", clnt_ip);
    log_action(user, msg);

    free(msg);

    while (running) {
        request req;

        get_request_header(clientSock, &(req.header), sizeof(req.header));

        switch (req.header.type) {
            case LIST:
                log_action(user, "LIST");
                perform_list(clientSock);
                break;
            case DIFF:
                log_action(user, "DIFF");
                perform_diff(clientSock);
                break;
            case PULL:
                log_action(user, "PULL");
                perform_pull(clientSock);
                break;
            case FETCH:
                log_action(user, "FETCH");
                perform_fetch(clientSock);
            case LEAVE:
                log_action(user, "LEAVE");
                perform_leave(clientSock);
                running = false;
                break;
        }

        /*
        if (strcmp(request, "LIST") == 0) {
            empty_list(client_list, free_file);
            read_directory(client_list);
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
            log_action(user, "User signed out.");

            close(clientSock);
            break;
        } else if(strcmp(request, "sendfile") == 0) {
            char *file_req = strtok(NULL, "\r\n");
            char *msg_to_log = NULL;
            int len = asprintf(&msg_to_log, "%s requested", file_req);
            log_action(user, msg_to_log);
            free(msg_to_log);
            send_file(file_req, clientSock);
            len = asprintf(&msg_to_log, "%s sent to user", file_req);
            free(msg_to_log);
            
        } else {
            send_message("Invalid request.\n", clientSock);
            // send back help or invalid command notification msg
        }
        */
    }

    return (NULL);
}

void get_client_ip(int clientSock, char *buf){
	socklen_t len;
	struct sockaddr_storage addr;
	char ip[INET_ADDRSTRLEN];
	int port;

	len = sizeof addr;
	getpeername(clientSock, (struct sockaddr*)&addr, &len);
	
	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
    
    snprintf(buf, sizeof(ip)+5, "%s:%i", ip, port);
}

void log_action(unsigned int user, char *message) {
    FILE *fp;
    char *buffer;
    int buffer_size;
    char *ts = timestamp();

    fp = fopen("log.txt", "a+");

    buffer_size = asprintf(&buffer, "[%s] %d: %s\n", ts, user, message);

    fwrite(buffer, sizeof(buffer[0]), buffer_size/sizeof(buffer[0]), fp);
    fclose(fp);
}
