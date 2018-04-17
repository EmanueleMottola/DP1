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
	struct in_addr inaddr;
	int s;
	int res, val1=0, val2=0;
	int result = inet_aton(argv[1], &inaddr);
	char buffer[BUFLEN], string[BUFLEN], string1[BUFLEN], string2[BUFLEN];

	if(!result){
		err_quit("Invalid address.\n\n");
	}
	if(sscanf(argv[2], "%" SCNu16, &tport_h));

	tport_n = htons(tport_h);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s < 0){
		fprintf(stderr, "Error in socket\n\n");
		exit(1);
	}

	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;

	res = connect(s, (const struct sockaddr*) &saddr, sizeof(saddr));
	if ( res < 0){
		fprintf(stderr, "errore connect");
		exit(1);
	}

	fprintf(stdout, "Connected..\n");
	fprintf(stdout, "Insert two integers:");
	fscanf(stdin, "%d %d %*s %*s ", &val1, &val2, string1, string2);

	if(strlen(string1)!=0){
		sprintf(string, "%s", string1);
		if(strlen(string2)!=0){
			sprintf(string, "%s%s", string1, 
	

	sprintf(string, "%d %d\r\n",val1 , val2);
	printf("%s", string);
	int n =	send(s, string, strlen(string)+1, 0);
	printf("Ho stampato %d bytes\n", n);
	n = recv(s, buffer, BUFLEN, 0);
	printf("ho ricevuto: %s", buffer);


	return 0;
}
