#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
#include <stdlib.h>         /* supports all sorts of functionality */
#include <unistd.h>         /* for close() */
#include <string.h>         /* support any string ops */
#include <stdbool.h>
#include <ctype.h>			/* for toupper() for command case formatting */

#pragma once

#define CMDLEN 5				/* for the cmd and the \n */
#define INPUT_BUFFER_MAX 80		/* max length of input buffer */