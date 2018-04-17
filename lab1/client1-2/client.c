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

char* prog_name = "client";

int main(int argc, char* argv[]){

	uint32_t taddr_n;
	uint16_t tport_n, tport_h;
	struct sockaddr_in saddr;
	struct in_addr IPaddr;
	int s;
	int res;
	int address;

	address = inet_aton(argv[1], &IPaddr);
	tport_h = atoi(argv[2]);
	tport_n = htons(tport_h);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s<0){
		err_sys("socket() non funziona, scemo\n\n");
		exit(1);
	}

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr = IPaddr;

	connect(s, (const struct sockaddr*) &saddr, sizeof(saddr));
	printf("Connected to server\n");

	close(s);



	return 0;
}
