#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <rpc/xdr.h>
#include <signal.h>
#include "errlib.h"
#include "sockwrap.h"
#include "lib.h"

#define BUFLEN 128
#define MAXBUF 255
#define N 100

char *prog_name;

void receive_file(char *filename, int buflen, int sockfd);
void manage_error(int sockfd);
void send_GET(char *buffer, int sockfd);
void send_cmdQUIT(int sockfd);

int flag;

void handler(int sig){
	if(sig == SIGUSR1)
		exit(0);
	else if(sig == SIGCHLD){
		exit(0);
	}	
}

int main(int argc, char* argv[]){
	int sockfd;
	struct addrinfo hints, *res = NULL;
	char receive_buffer[BUFLEN], filename[BUFLEN];
	int buflen=0, quit=0, abort=0, errorvalue;
	fd_set readset; int i=0;
	
	prog_name = argv[0];
	flag=0;
	
	sockfd = init_connection_tcp(hints, res, argv[1], argv[2]);
	
	unsigned int m = sizeof(buflen);
	Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*) &buflen, &m);
	buflen = buflen > MAXBUF ? MAXBUF : buflen;
	
	
	
	while(quit == 0){
		
		FD_ZERO(&readset);
		FD_SET(sockfd, &readset);
		FD_SET(0, &readset);
		printf("Inserire il comando:");	fflush(stdout);
		int n = Select(sockfd+1, &readset, NULL, NULL, NULL);
		
		if(n != 0){
			printf("(%d)-l'input è pronto.\n", i++);fflush(stdout);
			if(FD_ISSET(0, &readset)){
				memset(receive_buffer, 0, BUFLEN);
				if(fgets(receive_buffer, BUFLEN-1, stdin) != NULL){
					switch(receive_buffer[0]){
						case 'G': send_GET(receive_buffer, sockfd);
							break;
						case 'Q': quit=1; printf("Quit..\n\n");
							break;
						case 'A': abort=1; printf("Abort..\n\n");
							break;
						default: printf("Unable to manage the request.\n\n");
							break;
					}
					if(abort == 1){
						exit(1);
					}
				}
				else{
					printf("fgets torna NULL\n termina"); exit(1);
				}
			}
			else if(FD_ISSET(sockfd, &readset)){
				printf("(%d)-il socket è pronto.\n", i++);fflush(stdout);
				Getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &errorvalue, &m);
				if (errorvalue != 0) {
					fprintf(stdout, "Server closed connection\n\n");
					Close(sockfd);
					break;
				}
				strcpy(filename, receive_buffer+4);
				memset(receive_buffer, 0, BUFLEN);
				Readn(sockfd, receive_buffer, 1);
			
				switch(receive_buffer[0]){
					case '-': manage_error(sockfd);		
						break;
					case '+': receive_file(filename, buflen, sockfd);
						break;
					default: fprintf(stdout, "Unexpected receive, shutdown from the server\n"); abort = 1; Close(sockfd); break;
				}
			}
			else{
				printf("The select unlocked but unhandled case...\nexit\n\n"); exit(1);
			}
		}
		else{
			fprintf(stdout, "timeout occurred\n\n");
		}
		
	}
	if(quit == 1)
		send_cmdQUIT(sockfd);
	
	return 0;
}

void send_GET(char *buffer, int sockfd){
	char filename[BUFLEN];
	
	sscanf(buffer, "%*s %s", filename);
	printf("%s\n", filename);
	writen(sockfd, "GET ", 4);
	writen(sockfd, filename, strlen(filename));
	writen(sockfd, "\r\n", 2);
	return;
}

void send_cmdQUIT(int sockfd){
	printf("Mando la QUIT\n\n");
	writen(sockfd, "QUIT\r\n", 6);
	return;
}


void receive_file(char *filename, int buflen, int sockfd){
	
	uint32_t n_file_dim = 0, h_file_dim=0;
	uint32_t n_last_mod = 0, h_last_mod = 0;
	char buffer[buflen];
	char letter[2];
	fd_set readset;
	
	
	Readn(sockfd, buffer, 4);
	
	Readn(sockfd, &n_file_dim, 4);
	h_file_dim = ntohl(n_file_dim);
	
	Readn(sockfd, &n_last_mod, 4);
	h_last_mod = ntohl(n_last_mod);
	
	
	filename[strlen(filename)-1] = '\0';
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


void manage_error(int sockfd){
	
	fprintf(stdout, "Error received..\nClose\n");
	Close(sockfd);
	
	return;
}



