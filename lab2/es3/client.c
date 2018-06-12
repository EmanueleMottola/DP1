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

#define FILENAMELEN 255

char *prog_name;


int main(int argc, char* argv[]){
	
	struct addrinfo hints, *res;
	int sockfd, buflen, request_len;
	prog_name = argv[0];
	char trash[FILENAMELEN];
	char *request, *buf;
	struct timeval timeout;
	fd_set readset;
	uint32_t n_file_dim = 0, h_file_dim=0;
	uint32_t n_last_modification = 0, h_last_modification = 0;

	//struct sockaddr_in addr;
	prog_name = argv[0];

	

	if(argc<2){
		fprintf(stdout, "usage: %s <server_address> <server_port>\n"
		 "where server_address can be IPv4 or IPv6.\n", prog_name);
		exit(1);
	}

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_NUMERICHOST;
	
	printf("argv[1]: %s, argv[2]: %s\n", argv[1], argv[2]);

	Getaddrinfo(argv[1], argv[2], &hints, &res);

	sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	unsigned int m = sizeof(buflen);
	Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*) &buflen, &m);
	
	buf = malloc( (int) buflen/5 * sizeof(char));
	if(buf == NULL){
		fprintf(stdout, "Allocation error\n\n");
		exit(1);
	}
	printf("buflen: %d\n", strlen(buf));

	Connect(sockfd, (const struct sockaddr*) res->ai_addr, sizeof(struct sockaddr));

	//send the GET request

	
	writen(sockfd, "GET ", 4);
	writen(sockfd, argv[3], strlen(argv[3]))
	writen(sockfd, "\r\n", 2);

	FD_ZERO(&readset);
	FD_SET(sockfd, &readset);
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	Select(sockfd+1, &readset, NULL, NULL, &timeout);
	
	Readn(sockfd, buf, 2048);
	printf("buflen: %d\n", strlen(buf));
	int i=0;
	while(i<strlen(buf)){
		printf("i:%d", i);
		printf("\t%c\n", buf[i++]);
	}

	int r;
	if((r = sscanf(buf, "%5s\r\n%4u%4u", trash,  &n_file_dim, &n_last_modification )) != 3){
		printf("Errore numero di risposta, %d\n", r);
		exit(3);
	}
	printf("trash: %s", trash);

	h_last_modification = ntohl(n_last_modification);
	h_file_dim = ntohl(n_file_dim);

	printf("File dimension: %u, Last modification: %u\n", h_last_modification, h_file_dim);

	//prepare the file descriptor
	int fd = open(argv[3], O_WRONLY | O_APPEND | O_CREAT);
	if(fd == -1){
		fprintf(stdout, "Can't open the file\n\n");
		exit(2);
	}

	printf("receiving the file..\n");

	int nbytes=0;
	while( nbytes < h_file_dim ){
		nbytes += Readn(sockfd, buf, buflen);
		printf("buf: %d, filedim: %d\n", nbytes, h_file_dim);
		Writen(fd, buf+8, buflen);
	}
	
	Writen(sockfd, "QUIT\r\n", 6);

	Close(sockfd);
	Close(fd);	
	

	
	
	return 0;
}







