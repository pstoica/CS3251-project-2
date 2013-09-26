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

#define RCVBUFSIZE 512      /* The receive buffer size */
#define SNDBUFSIZE 512      /* The send buffer size */
#define MAXPENDING 5        /* Maximum outstanding connection requests */


void DieWithError(char *errorMessage);          /* Error handler */
void HandleTCPClient(int clntSocket);           /* TCP client handling function */
int SetupTCPServerSocket(unsigned short port); /* Create TCP server socket */
int AcceptTCPConnection(int servSock);          /* Accept TCP connection request */

int main(int argc, char *argv[]) {
    if (argc < 3) {
        DieWithError("Parameter(s): <Timeout (secs.)> <Port/Service1> ...")
    }

    fd_set sockSet;               /* set of socket descriptors for select() */
    long timeout = atol(argv[1]); /* argument 1: Timeout */
    int noPorts = argc - 2;       /* argument 2: Number of ports */
    int servSock[noPorts];        /* list of sockets for incoming connections */
    int maxDescriptor = -1;       /* initialize maxDescriptor for select() */

    struct timeval selTimeout; /* timeout for select() */

    for (int port = 0; port < noPorts; port++) {
        servSock[port] = SetupTCPServerSocket(argv[port + 2]);

        if (servSock[port] > maxDescriptor) {
            maxDescriptor = servSock[port];
        }
    }

    while (true) {
        /* zero socket descriptor vector and set for server sockets */
        /* must occur for every select() call */
        FD_ZERO(&sockSet);

        /* set timeout from command line arguments */
        selTimeout.tv_sec = timeout;
        selTimeout.tv_usec = 0;

        for (int port = 0; port < noPorts; port++) {
            FD_SET(servSock[port], &sockSet);
        }

        if (select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0) {
            printf("No requests for %ld secs... Server still alive\n", timeout);
        } else {
            // process connection requests
            for (int port = 0; port < noPorts; port++) {
                if (FD_ISSET(servSock[port], &sockSet)) {
                    printf("Request on port %d: ", port);
                    HandleTCPClient(AcceptTCPConnection(servSock[port]));
                }
            }
        }
    }

    for (int port = 0; port < noPorts; port++) {
        close(servSock[port]);
    }

    exit(0);
}

int SetupTCPServerSocket(const char *service) {
    int servSock;
    struct sockaddr_in servAddr;      /* Local address */

    /* Construct local address structure*/
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(service));

    /* Bind to local address structure */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        DieWithError("bind() failed")
    }

    /* Listen for incoming connections */
    if (listen(servSock, 5) < 0) {
        DieWithError("listen() failed")
    }

    return servSock;
}

int AcceptTCPConnection(int servSock) {

}

void HandleTCPClient(int clientSock) {

}

void DieWithError(char *message) {
    perror(message);
    exit(1);
}