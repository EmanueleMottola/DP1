#ifndef _LIBH_

#define _LIBH_

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

int init_connection_tcp(struct addrinfo hints,
                        struct addrinfo *res,
                        char* address,
                        char* port);

int init_connection_udp(const char *address,
                        const char *port,
                        socklen_t *addrlenp);

#endif
