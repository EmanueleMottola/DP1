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
#include "../errlib.h"
#include "../sockwrap.h"

int init_connection_tcp(struct addrinfo hints,
                        struct addrinfo *res,
                        char* address,
                        char* port){

    bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_NUMERICHOST;

    Getaddrinfo(address, port, &hints, &res);

    int sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    Connect(sockfd, (const struct sockaddr*) res->ai_addr, sizeof(struct sockaddr));

    return sockfd;
}

int init_connection_udp(const char *address,
                        const char *port,
                        socklen_t *addrlenp){
    int     sockfd;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    Getaddrinfo (address, port, &hints, &res);
    ressave = res;

    do {
        sockfd = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd >= 0)
            break;       /* success */
    } while ( (res = res->ai_next) != NULL);

    if (res == NULL)           /* errno from final socket() or bind() */
        err_sys ("udp_client error for %s, %s", address, port);
    if (addrlenp)
        *addrlenp = res->ai_addrlen;    /* return size of protocol address */

    freeaddrinfo (ressave) ;

    return (sockfd);
}