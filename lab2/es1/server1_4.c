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
#include "errlib.h"
#include "sockwrap.h"

#define BUFLEN 128
#define NUMCLIENT 10

char *prog_name;

void die(char *s){
	perror(s);
	exit(1);
}

typedef struct clients{

	struct sockaddr_in clientaddr;
	int num_req;

} CLIENT;

int main(int argc, char* argv[]){

	prog_name = argv[0];
	struct sockaddr_in inaddr, clientaddr;
	char buf[BUFLEN];
	int s;
	uint16_t PORT;
	unsigned int clientaddrlen = sizeof(clientaddr);
	CLIENTS requests[NUMCLIENT];

	init_requests(&requests, NUMCLIENTS);
	

	sscanf(argv[1], "%" SCNu16, &PORT);

	//create an UDP socket
	s = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//zero out the structure
	memset( (char*) &inaddr, 0, sizeof(inaddr));

	inaddr.sin_family = AF_INET;	
	inaddr.sin_port = htons(PORT);
	inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 
	printf("server on - addr %s:%d\n", inet_ntoa(inaddr.sin_addr), PORT);
	printf("waiting for connections...\n");

	//bind socket to port
	Bind(s, (struct sockaddr*) &inaddr, sizeof(inaddr) );
	

	while(1){

		int n = Recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*) &clientaddr, &clientaddrlen);

		//firt request of the client
		if( check(inet_ntoa(clientaddr.sin_addr), requests, NUMCLIENTS) == -1){
			insert_requests(clientaddr, requests);
		}
		//not first request
		else{
			todo = manage_request();
			if(todo != -1){
				printf("data received...\nbuf: %s\n", buf);

				if(n>0){
					Sendto(s, buf, BUFLEN, 0, (struct sockaddr *) &clientaddr, clientaddrlen);
					printf("data sent back..\n");}
				else
					printf("errore recv\n");
				}
		}
	}

	Close(s);

	printf("connection closed\n");

	return 0;
}


void init_requests(CLIENT **requests, int len){

	int i;
	for(i=0; i<len; i++){
		requests[i]->req = 0;
		memset((char*)request[i]->clientaddr, 0, sizeof(request[i]->clientaddr) );
	}
	return;
}

int check(char* str, CLIENT **requests, int len){

	for(int i=0; i<len; i++){
		if(strcmp(str, inet_ntoa(requests[i]->clientaddr.sin_addr) == 0)
			return 0; 
	}
	return -1;
}



