
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
#include <sys/wait.h>
#include <signal.h>
#include "../errlib.h"
#include "../sockwrap.h"

#define FILENAMELEN 255
#define N_LISTEN 15
#define GETLEN 300
#define HEADER 13
#define MAXBUF 2048
#define IPLENGTH 39
#define MAXPROCESSES 8

char *prog_name;

int send_file(char * filename, int buflen, int connfd);
void fatal_error(char *string, int error_number);
void error_msg(char *problem, int connfd);
void serve_client(int connfd, int buflen);

int main(int argc, char* argv[]){

	struct addrinfo hints, *res;
	uint16_t port;
	int buflen, connfd;
	
	
	if(argc < 2 || argc > 3){
		fatal_error("Wrong number of arguments\nUsage: %s <port>", 1);
	}
	
	sscanf(argv[1], "%" SCNu16, &port);
	if(port < 1025 || port > 65535){
		fatal_error("Wrong port number, allowed interval: 1025 - 65535", 2);
	}
	
	prog_name = argv[0];
	
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	Getaddrinfo(NULL, argv[1], &hints, &res);
	
	int sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	unsigned int m = sizeof(buflen);
	Getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void*) &buflen, &m);
	buflen = buflen > MAXBUF ? MAXBUF : buflen;

	Bind(sockfd, (struct sockaddr*) res->ai_addr, res->ai_addrlen);
	
	Listen(sockfd, N_LISTEN);
	
	while(1){
		printf("Waiting for connections...\n\n");
			
		connfd = Accept(sockfd, NULL, NULL);
		if(connfd < 0){
			fatal_error(strerror(errno), 4);
		}
		
		if( fork() == 0){
			Close(sockfd);
			serve_client(connfd, buflen);
			exit(0);
		}
		signal(SIGCHLD, SIG_IGN);
		Close(connfd);
	}
	
	return 0;
}

void serve_client(int connfd, int buflen){
	
	fd_set readset;
	struct timeval timeout;
	int read=0, errorvalue=0, r;
	char filename[FILENAMELEN], command[5];
	unsigned int m = sizeof(buflen);
	
	char* buf = malloc( buflen * sizeof(char));
	if(buf == NULL){
			fatal_error("Allocation error", 3);
	}
	
	do{
		FD_ZERO(&readset);
		FD_SET(connfd, &readset);
		timeout.tv_sec = 2; timeout.tv_usec = 0;
		r = select(connfd+1, &readset, NULL, NULL, &timeout);
		if(r == EBADF){
			fprintf(stdout, "Client closed connection\n\n");
			Close(connfd);
			break;
		}
		if(r != 0) {
			Getsockopt (connfd, SOL_SOCKET, SO_ERROR, &errorvalue, &m);
			if (errorvalue != 0) {
				fprintf(stdout, "Client closed connection\n\n");
				Close(connfd);
				break;
			}
			read = Read(connfd, buf, buflen);
		}
		else {
			fprintf(stdout, "timeout from the client, connection closed..\n"); 
			Close(connfd); 
			read=0;
			break;
		}
		
		
		if(strncmp(buf,"QUIT\r\n", 6) == 0){
			read=0;
			fprintf(stdout, "Client closed connection-0\n\n");
			Close(connfd);
			break;
		}
		else{
			int n;
			if((n = sscanf(buf, "%s %s\r\n", command, filename)) == 2){
				printf("command: <%s>, filename: <%s>\n", command, filename);
				if(strncmp(command, "GET", 3) == 0){
					if(send_file(filename, buflen, connfd))
						printf("File %s sent..\n", filename);
					else{
						printf("Unable to send the file..\n\n");break;
					}
				}
				else{
					error_msg("Wrong request format.\nUse GET <filename>\\r\\n", connfd);break;
				}
			}
			else{
				error_msg("Wrong request format.\nWrong number of elements", connfd);break;
			}
		}
	}
	while(read > 0);
		
	return;
}


int send_file(char * filename, int buflen, int connfd){
	
	char buffer[buflen], size[4], time[4];
	struct stat filestat;
	int bytes_read=0;
	int to_read=0;
	int errorvalue=0;
	int totbytessent = 0;
	
	int fd = open(filename, O_RDONLY);
	if(fd == -1){
		fprintf(stdout, "File opening error...\n\n");
		return 0;
	}
	
	fstat(fd, &filestat);
	
	uint32_t filesize = (uint32_t) filestat.st_size;
	filesize = htonl(filesize);
	
	uint32_t filetime = (uint32_t)filestat.st_mtime;
	filetime = htonl(filetime);
	
	memcpy(size, &filesize, 4);
	memcpy(time, &filetime, 4);
	
	unsigned int m = sizeof(buflen);
	Getsockopt (connfd, SOL_SOCKET, SO_ERROR, &errorvalue, &m);
	if (errorvalue != 0) {
		fprintf(stdout, "Client closed connection-1\n\n");
		Close(connfd);
		return 0;
	}
	
	int bytes_sent = writen(connfd, "+OK\r\n", 5);
	
	
	if(bytes_sent == -1 || bytes_sent == EBADF){
		fprintf(stdout, "(1)-Can't send bytes on socket..bad file descr\n\n");
		Close(connfd);
		return 0;
	}
	
	
	Getsockopt (connfd, SOL_SOCKET, SO_ERROR, &errorvalue, &m);
	if (errorvalue != 0) {
		fprintf(stdout, "Client closed connection-2\n\n");
		Close(connfd);
		return 0;
	}
	
	
	totbytessent += bytes_sent;
	
	bytes_sent = writen(connfd, size, 4);
	if(bytes_sent == -1 || bytes_sent == EBADF){
		fprintf(stdout, "(1)-Can't send bytes on socket..\n\n");
		Close(connfd);
		return 0;
	}
	totbytessent += bytes_sent;
	

	Getsockopt (connfd, SOL_SOCKET, SO_ERROR, &errorvalue, &m);
	if (errorvalue != 0) {
		fprintf(stdout, "Client closed connection-3\n\n");
		Close(connfd);
		return 0;
	}
	
	bytes_sent = writen(connfd, time, 4);
	if(bytes_sent == -1 || bytes_sent == EBADF){
		fprintf(stdout, "(1)-Can't send bytes on socket..\n\n");
		Close(connfd);
		return 0;
	}
	totbytessent += bytes_sent;
	
	while(totbytessent < filestat.st_size){
		to_read = filestat.st_size < buflen ? filestat.st_size : buflen;
		bytes_read = Readn(fd, buffer, to_read);
		//printf("bytes_read: %d", bytes_read);
		if(bytes_read == -1){
			fprintf(stdout, "Can't read from file..\n\n");
			return 0;
		}
		
		Getsockopt (connfd, SOL_SOCKET, SO_ERROR, &errorvalue, &m);
		if (errorvalue != 0) {
			fprintf(stdout, "Client closed connection-4\n\n");
			Close(connfd);
			return 0;
		}
		bytes_sent = writen(connfd, buffer, bytes_read);
		totbytessent += bytes_sent;
		//printf("bytes_sent: %d\n", bytes_sent);
		//sleep(1);
	}
	
	
	return 1;
}

void fatal_error(char *string, int error_number){
	fprintf(stdout, "[error] - %s\n", string);
	exit(error_number);
}

void error_msg(char *problem, int connfd){
	char finale_send[7];
	
	sprintf(finale_send, "-ERR\r\n");
	write(connfd, finale_send, 7);
	
	printf("%s", problem);
	
	Close(connfd);
	
	return;
}






