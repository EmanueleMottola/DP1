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
#include <sys/stat.h>
#include "errlib.h"
#include "sockwrap.h"

#define FILENAMELEN 255
#define N_LISTEN 15

char *prog_name;

void send_file(char * filename, int buflen, int connfd);
void error(char *problem, int connfd);

int main(int argc, char* argv[]){

	struct sockaddr_in server_addr, client_addr;
	uint16_t port;
	int buflen, connfd;
	socklen_t addr_len = sizeof(client_addr);
	char filename[FILENAMELEN], command[5];
	
	prog_name = argv[0];
	
	if(argc!=2){
		fprintf(stderr, "Wrong number of arguments\nUsage: %s <port>\n", prog_name);
		exit(1);
	}
	

	sscanf(argv[1], "%" SCNu16, &port);

	int sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	/*unsigned int m = sizeof(buflen);
	Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*) &buflen, &m);*/
	buflen = 512;
	
	
	char* buf = malloc( buflen * sizeof(char));
	if(buf == NULL){
			fprintf(stdout, "Allocation error\n\n");
			exit(2);
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	unsigned int serverlen = sizeof(server_addr);
	Bind(sockfd, (struct sockaddr*) &server_addr, serverlen);
	fprintf(stdout, "Server address %s:%d..\n", inet_ntoa(server_addr.sin_addr), port);

	printf("listening on socket...\n");
	Listen(sockfd, N_LISTEN);

	while(1){
		printf("Waiting for connections...\n");
		connfd = Accept(sockfd, (struct sockaddr*) &client_addr, &addr_len);
		if(connfd <0){
			printf("ERROE: %s", strerror(errno));
		}
		fprintf(stdout, "Connection from %s..\n", inet_ntoa(client_addr.sin_addr));
		Read(connfd, buf, buflen);
		printf("Received from client:%s\n", buf);
		
		if(strcmp(buf,"QUIT\r\n") == 0){
				fprintf(stdout, "Client closed connection\n\n");
				Close(connfd);
		}
		else{
			int n;
			if((n = sscanf(buf, "%s %s\r\n", command, filename)) == 2){
				printf("command: <%s>, filename: <%s>\n", command, filename);
				if(strcmp(command, "GET") == 0){
					printf("sono nella get\n");
					send_file(filename, buflen, connfd);
					printf("File %s sent..\n", filename);
				}
				else{
					error("Wrong request format.\nUse GET <filename>\r\n", connfd);
				}
			}
			else{
				error("Wrong request format.\nWrong number of elements", connfd);
			}
		}

	}
	

	return 0;
}


void send_file(char * filename, int buflen, int connfd){
	
	
	
	return;
}

void error(char *problem, int connfd){
	char finale_send[7];
	
	sprintf(finale_send, "-ERR\r\n");
	write(connfd, finale_send, 7);
	
	printf("%s", problem);
	
	Close(connfd);
	
	return;
}



















