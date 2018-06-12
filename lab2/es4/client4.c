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
#include <rpc/xdr.h>
#include "errlib.h"
#include "sockwrap.h"

#define BUFLEN 128

char* prog_name = "client";

int main(int argc, char* argv[]){

	uint16_t tport_n, tport_h;
	struct sockaddr_in saddr;
	struct in_addr inaddr;
	int s;
	int res, val1=0, val2=0;
	int result = inet_aton(argv[1], &inaddr);
	char buffer[4];

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
	fscanf(stdin, "%d %d", &val1, &val2);

	//xdr declaration
	XDR xdrs_in;
	XDR xdrs_out;
	FILE *fdstream_in, *fdstream_out;
	
	//xdr streams
	fdstream_out = fdopen(s, "w");
	fdstream_in = fdopen(s, "r");
	
	//xdr creation for writing the two values
	xdrstdio_create(&xdrs_out, fdstream_out, XDR_ENCODE);
	xdr_int(&xdrs_out, &val1);
	xdr_int(&xdrs_out, &val2);
	
	fflush(fdstream_out);
	
	xdr_destroy(&xdrs_out);
	
	
	//xdr creation for reading 
	xdrstdio_create(&xdrs_in, fdstream_in, XDR_DECODE);
	xdr_int(&xdrs_in, &val1);
		
	xdr_destroy(&xdrs_out);
	
	printf("ho ricevuto: %d\n", val1);


	return 0;
}
