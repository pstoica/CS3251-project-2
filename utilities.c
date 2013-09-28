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
        received_size = recv(clientSock, buffer, RECEIVE_BUFFER_SIZE, 0);
        if (received_size < 0) {
            free(message);
            return 0;
        } else if (received_size > 0) {
            int i = 0;

            for (i = 0; i < received_size; i++) {
                message[++message_size] = buffer[i];

                /* if our message size is about to go over our capacity,
                   increase the string size and reallocate */
                if (message_size >= capacity) {
                    capacity += RECEIVE_BUFFER_SIZE;
                    message = realloc(message, capacity);
                }
            }

            memset(buffer, 0, RECEIVE_BUFFER_SIZE);
        }
    } while (!is_valid(message) && received_size > 0);

    return message;
}

int is_valid(char *message) {
    int length = strlen(message);

    if (length < 1) return false;
    return (message[length - 1] == '\n');
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