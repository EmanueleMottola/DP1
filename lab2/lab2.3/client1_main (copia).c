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

#define FILENAMELEN 255
#define GETLEN 300
#define HEADER 13
#define MAXBUF 2048

char *prog_name;

void receive_file(char *filename, int buflen, int sockfd);
void manage_error(char *buffer, int sockfd);

int main(int argc, char* argv[]){
	
	struct addrinfo hints, *res;
	int sockfd, buflen;
	char buffer[GETLEN];
	struct timeval timeout;
	fd_set readset;
	
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
	
	//printf("argv[1]: %s, argv[2]: %s\n", argv[1], argv[2]);

	Getaddrinfo(argv[1], argv[2], &hints, &res);

	sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	unsigned int m = sizeof(buflen);
	Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*) &buflen, &m);
	buflen = buflen > MAXBUF ? MAXBUF : buflen;
	

	Connect(sockfd, (const struct sockaddr*) res->ai_addr, sizeof(struct sockaddr));
	
	
	int n_files = argc - 3;
	while(n_files > 0){
		
		if(strcmp( argv[argc - n_files], "QUIT")==0){
			sprintf(buffer, "QUIT\r\n");
		}
		else{
			sprintf(buffer, "GET %s\r\n", argv[argc-n_files]);
		}
		
		writen(sockfd, buffer, strlen(buffer));
		
		FD_ZERO(&readset);
		FD_SET(sockfd, &readset);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		int n = Select(sockfd+1, &readset, NULL, NULL, &timeout);
		
		if( n!=0 ){
			Readn(sockfd, buffer, 1);
			
			switch(buffer[0]){
				case '-': manage_error(buffer, sockfd);		
					break;
				case '+': receive_file(argv[argc - n_files], buflen, sockfd);
					break;
				default: fprintf(stdout, "Unexpected receive, shutdown");
			}
		}
		else{
			fprintf(stdout, "(%s) - timeout occurred\n\n", prog_name);
			break;
		}
			
		n_files--;
	}
		
	Shutdown(sockfd, 1);
	
	return 0;
}

void receive_file(char *filename, int buflen, int sockfd){
	
	uint32_t n_file_dim = 0, h_file_dim=0;
	uint32_t n_last_mod = 0, h_last_mod = 0;
	char buffer[buflen];
	
	Readn(sockfd, buffer, 4);
	
	Readn(sockfd, &n_file_dim, 4);
	h_file_dim = ntohl(n_file_dim);
	
	Readn(sockfd, &n_last_mod, 4);
	h_last_mod = ntohl(n_last_mod);
	
	

	//prepare the file descriptor
	int fd = open(filename, O_WRONLY | O_CREAT, 0666);
	if(fd == -1){
		fprintf(stdout, "Can't open the file\n\n");
		exit(2);
	}
	
	printf("receiving the file %s\n", filename);
	printf("\tFile dimension: %u\n\tLast modification: %u\n", h_file_dim, h_last_mod );

	uint32_t nbytes_tot=0, nbytes_read=0, to_read=0;
	
	while( nbytes_tot < h_file_dim ){
		
		to_read = h_file_dim < buflen ? h_file_dim : buflen;
		
		nbytes_read = Read(sockfd, buffer, to_read);
		
		nbytes_tot += nbytes_read;//printf("tot:%d\tread: %d\th_file_dim: %d\n", nbytes_tot, nbytes_read, h_file_dim);
		Write(fd, buffer, nbytes_read);
	}
	
	
	
	Close(fd);
	
	return;
}

void manage_error(char *buffer, int sockfd){
	
	fprintf(stdout, "Error received..\nClose\n");
	Close(sockfd);
	
	return;
}





