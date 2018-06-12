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
#include "errlib.h"
#include "sockwrap.h"

int init_server_tcp(struct addrinfo hints,
                    struct addrinfo *res,
                    char* port){
							
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    Getaddrinfo(NULL, port, &hints, &res);

    int sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    Bind(sockfd, (struct sockaddr*) res->ai_addr, res->ai_addrlen);
    
    Listen(sockfd, 10);

    return sockfd;
}
