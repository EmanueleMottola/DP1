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
#include "errlib.h"
#include "sockwrap.h"

#define BUFLEN 32

char* prog_name = "client1_4";

int main(int argc, char* argv[]){

	uint16_t t_porth;
	struct sockaddr_in inaddr, inaddr2;
	char buf[BUFLEN];
	int s, n, i=0;
	unsigned int sl = sizeof(inaddr2);
	struct timeval timeout;
	fd_set readset;


	sscanf(argv[2], "%" SCNu16, &t_porth);
	strcpy(buf, argv[3]);

	//create an UDP socket
	s = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	printf("buf: %s\n", buf);

	//zero out the structure
	memset( (char*) &inaddr, 0, sizeof(inaddr));

	inaddr.sin_family = AF_INET;	
	inaddr.sin_port = htons(t_porth);
	Inet_pton(AF_INET, argv[1], &inaddr.sin_addr);

	Sendto(s, buf, BUFLEN, 0, (struct sockaddr *) &inaddr, sl);

	FD_ZERO(&readset);
	FD_SET(s, &readset);
	timeout.tv_sec = 3; timeout.tv_usec = 0;

	while(i<5){
		timeout.tv_sec = 3; timeout.tv_usec = 0;
		printf("tentativo %d\n", i);
		n = Select(s+1, &readset, NULL, NULL,&timeout);

		if(n>0){
			memset(buf, 0, BUFLEN);
			printf("buf: %s\n", buf);
	
			Recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &inaddr2, &sl);
			printf("buf: %s\n", buf);
		}
		else{
 			printf("timeout\n");i++;
		}
		
	}
	

	close(s);
	return 0;
}



