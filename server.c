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

static int users = 0;

void *thread_main(void *args);   /* Main program of a thread */

void perform_list(int sock, request *req, list *file_list);
void perform_diff(int sock, request *req, list *file_list, list *client_list, list *diff_list);
void perform_pull(int sock, request *req, list *file_list);
void perform_fetch(int sock, request *req, int user);
void perform_leave(int sock, request *req);

void log_action(unsigned int user, char *message);
void get_client_ip(int clientSock, char *buf);

int main(int argc, char *argv[]) {
    int servSock;
    int clientSock;
    unsigned short port;
    pthread_t threadID;
    struct thread_args *threadArgs;

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

void perform_list(int sock, request *req, list *file_list) {
    response res;
    char *list;

    empty_list(file_list, free_file);
    read_directory(file_list);
    list = traverse_to_string(file_list, file_to_string);

    res.header.size = (sizeof(char) * (strlen(list) + 1));
    res.data = list;

    send_data(sock, &(res.header), sizeof(res.header));
    send_data(sock, res.data, res.header.size);
}

void perform_diff(int sock, request *req, list *file_list, list *client_list, list *diff_list) {
    char *list;
    response res;

    list = get_data(sock, req->header.size);

    empty_list(client_list, free_file);
    deserialize_list(client_list, list);

    empty_list(file_list, free_file);
    read_directory(file_list);

    empty_list(diff_list, free_file);
    traverse_diff(file_list, client_list, diff_list, file_comparator);

    list = traverse_to_string(diff_list, file_to_string);

    res.header.size = (sizeof(char) * (strlen(list) + 1));
    res.data = list;

    send_data(sock, &(res.header), sizeof(res.header));
    send_data(sock, res.data, res.header.size);
}

/* just use perform_diff */
void perform_pull(int sock, request *req, list *file_list) {
    printf("PERFORM PULL\n");
}

void perform_fetch(int sock, request *req, int user) {
    response res;
    char *file_req;
    char *msg_to_log;
    int len;

    file_req = get_data(sock, req->header.size);
    len = asprintf(&msg_to_log, "%s requested", file_req);
    printf("[User %i] FETCH %s requested\n", user, file_req);

    log_action(user, msg_to_log);
    free(msg_to_log);

    send_file(sock, &res, file_req);

    len = asprintf(&msg_to_log, "%s sent to user", file_req);
    printf("[User %i] FETCH %s Complete\n", user, file_req);
    log_action(user, msg_to_log);
    free(msg_to_log);
}

void perform_leave(int sock, request *req) {
    close(sock);
}

void *thread_main(void *threadArgs) {
    bool running = true;
    int clientSock = ((struct thread_args *) threadArgs)->sock;

    pthread_detach(pthread_self());
    free(threadArgs);

    list *file_list = create_list();
    list *client_list = create_list();
    list *diff_list = create_list();

    int user = ++users;
    char clnt_ip[INET_ADDRSTRLEN+5]; 
    get_client_ip(clientSock, clnt_ip);
    char *msg;

    asprintf(&msg, "User %i signed in (%s)", user, clnt_ip);
    log_action(user, msg);

    free(msg);

    while (running) {
        request req;

        get_request_header(clientSock, &(req.header), sizeof(req.header));

        switch (req.header.type) {
            case LIST:
                log_action(user, "LIST");
                printf("[User %i] LIST\n", user);
                perform_list(clientSock, &req, file_list);
                break;
            case DIFF:
                log_action(user, "DIFF");
                printf("[User %i] DIFF\n", user);
                perform_diff(clientSock, &req, file_list, client_list, diff_list);
                break;
            case PULL:
                log_action(user, "PULL");
                printf("[User %i] PULL\n", user);
                perform_diff(clientSock, &req, file_list, client_list, diff_list);
                break;
            case FETCH:
                perform_fetch(clientSock, &req, user);
                break;
            case LEAVE:
                log_action(user, "LEAVE");
                printf("[User %i] LEAVE\n", user);
                perform_leave(clientSock, &req);
                running = false;
                break;
        }
    }

    empty_list(file_list, free_file);
    empty_list(client_list, free_file);
    empty_list(diff_list, free_file);
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

    fp = fopen("activity.log", "a+");

    buffer_size = asprintf(&buffer, "[%s] %d: %s\n", ts, user, message);

    fwrite(buffer, sizeof(buffer[0]), buffer_size/sizeof(buffer[0]), fp);
    fclose(fp);
}
