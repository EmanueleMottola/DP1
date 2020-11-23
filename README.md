# Socket Programming

## Select function.

The select function is used in order to manage I/O multilexing.
Prototype of this function is:

*int select(int maxfd, fd_set *readset, fd_set *writeset, fd_set *exceptionset, struct timeval *timeout);*

* **maxfd** indicates the maximum number of file descriptor to wait on.
* **readset** indicates the set of file descriptors to wait on for reading.
* **writeset** indicates the set of file desciptors to wait on for writing.
* **exceptionset** indicates the set of file descriptors to wait on for exceptions.
* **timeout** is the amount of time to wait. It can be:
    * NULL -> wait forever
    * value -> to wait <value> amount of time
    * 0 if we want to return immediately after the check.

The readset is ready if:
1. the socket has some data to be read.
2. an error occurred in on the socket.
3. The read half of the connection is closed.
4. The socket is a listening socket and the number of completed connection is nonzero.

The writeset is ready if:
1. The socket write is closed.
2. The connect was performed or it encountered an error.
3. Write buffer is ready
4. There is an error on the socket.

#### Return value

* **-1** on error.
* **0** if the connection is closed or timeout exprired.
* **> 0** normally.

#### Code example

``````
void str_cli(FILE *FP, int sockfd){

  int maxfd1, stdineof;  
  fd_set rset;char buf[MAXLINE];  
  int n;  

  stdineof = 0;  
  FD_ZERO(&rset);  
  for(;;) {  
    if(stdineof == 0)  
      FD_SET(fileno(fp), &rset);
    FD_SET(sockfd, &rset);
    maxfd1 = max(fileno(fp), sockfd) +1;
    Select(maxfd1, &rset, NULL, NULL, NULL);

    if(FD_ISSET(sockfd, &rset)){ // socket is readable
      if((n = Read(sockfd, buf, MAXLINE)) == 0) {
        if(stdineof == 1)
          return;
        else
          err_quit("str_cli: server terminated prematurely");
      }
      Write(fileno(stdout), buf, n);
    }
    if(FD_ISSET(sockfd, &rset)){
      if((n = Read(fileno(fp), buf, MAXLINE)) == 0){
        stdineof = 1;
        Shutdown(sockfd, SHUT_WR);
        FD_CLR(fileno(fp), &rset);
        continue;
      }
      Writen(sockfd, buf, n);
    }
  }
}

``````
#### [server] Client memory - Best practice
 If we want to memorize the clients which ask for a connection to the server, it is necessary to handle a data structure (array) where the file descriptor numbers of the sockets are stored. In this way we are able to manage the Select on those sockets. This data structure must be set to -1 and when a new connection arrive, then the file descriptor number is stored in the data structure. When connection closes, then it is necessary to reset the array field to -1.

 ##### Code example
``````
int
3 main(int argc, char **argv)
4 {
5 int i, maxi, maxfd, listenfd, connfd, sockfd;
6 int nready, client[FD_SETSIZE];
7 ssize_t n;
8 fd_set rset, allset;
9 char buf[MAXLINE];
10 socklen_t clilen;
11 struct sockaddr_in cliaddr, servaddr;

12 listenfd = Socket(AF_INET, SOCK_STREAM, 0);

13 bzero(&servaddr, sizeof(servaddr));
14 servaddr.sin_family = AF_INET;
15 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
16 servaddr.sin_port = htons(SERV_PORT);

17 Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

18 Listen(listenfd, LISTENQ);

19 maxfd = listenfd; /* initialize */
20 maxi = -1; /* index into client[] array */
21 for (i = 0; i < FD_SETSIZE; i++)
22 client[i] = -1; /* -1 indicates available entry */

23 FD_ZERO(&allset);
24 FD_SET(listenfd, &allset);

25 for ( ; ; ) {
26  rset = allset; /* structure assignment */
27  nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
28  if (FD_ISSET(listenfd, &rset)) { /* new client connection */
29    clilen = sizeof(cliaddr);
30    connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
31    for (i = 0; i < FD_SETSIZE; i++)
32      if (client[i] < 0) {
33        client[i] = connfd; /* save descriptor */
34        break;
35      }
36    if (i == FD_SETSIZE)
37      err_quit("too many clients");
38    FD_SET(connfd, &allset); /* add new descriptor to set */
39    if (connfd > maxfd)
40      maxfd = connfd; /* for select */
41    if (i > maxi)
42      maxi = i; /* max index in client[] array */
43    if (--nready <= 0)
44      continue; /* no more readable descriptors */
45   }
46    for (i = 0; i <= maxi; i++) { /* check all clients for data */
47      if ( (sockfd = client[i]) < 0)
48        continue;
49      if (FD_ISSET(sockfd, &rset)) {
50        if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
51          /* connection closed by client */
52          Close(sockfd);
53          FD_CLR(sockfd, &allset);
54          client[i] = -1;
55        } else
56        Writen(sockfd, buf, n);
57        if (--nready <= 0)
58          break; /* no more readable descriptors */
59      }
60    }
61  }
``````

**********

## Name and Address Conversions

Names are easier to remember than numbers, for this reason it is necessary to introduce functions to convert between names and numeric values.

### Domain Name System (DNS)

To map between *hostnames* and *IP addresses*.  
* *__gethostbyname__*: maps hostname to IPv4.
* *__gethostbyaddr__*: maps IPv4 to hostname.

#### *gethostbyname* function

This function returns a struct hostent function, which is the structure corresponding to the hostname passed by parameter. The main limitation of this function is that is can only return IPv4 addresses.

Prototype:  
``````
struct hostent *gethostbyname(const char *hostname);
``````


The hostent structure is the following:   
``````
struct hostent{
  char *h_name;     //offical name of host
  char **h_aliases; //pointer to array of pointers to alias names
  int h_addrtype;   //host address type : AF_INET
  int h_length;     //length of address: 4
  char **h_addr_list; //ptr to array of ptrs with IPv4 addrs
};
``````
The hostname element can also be a dotted decimal address and so also htpr = gethostbyname("192.168.42.2"); will work.

#### *gethostbyadd* function

This function returns  a hostent  structure like the one highlighted above, whose we are interested in the canonical name filed (h_name).

#### *getservbyname* and *getservbyport* functions

Functions which tell the user which port is used for a know service. If the port number changes, we do not have to recompile the entire application but to change a line in the */etc/services* file.

Prototype:
``````
struct servent *getservbyname(const char *servname, const char *protoname);
``````
and the structure returned is the following one:
``````
struct servent{
  char *s_name;   //official service name
  char **s_aliases; // alias listenfd
  int s_port;
  int s_proto;
};
``````

#### *getaddrinfo* function
The previous functions only support IPv4. For this reason, *getaddrinfo* function was introduced: in order to support **also** IPv6.  
It supports IPv4 and IPv6 and the prototype of the function is:
``````
int getaddrinfo (const char* hostname, const char *service, const struct addrinfo hints, struct addrinfo **result);

``````
This fucntion handles both name-to-address and service-to-port resolutions.  
Parameters:
* **hostname**: it can be either a hostname or an address string (IPv4 and IPv6).
* **service**: it is both a service name or a port number.
* **hints**: it is either a NULL pointer or a pointer to an addrinfo structure that the caller fits with data type he is interested in finding. The members of this structure that can be set are:
  * **ai_flags**: AI_PASSIVE (server), AI_CANONNAME (the function returns the canonical name of the host), AI_NUMERICHOST() etc..
  * **ai_family**: it specifies the type of protocol to be used. Use AF_UNSPEC if you don't know which is the version of IP we are interested. Otherwise AF_INET and AF_INET6.
  * **ai_socktype**: SOCK_STREAM or SOCK_DGRAM.  


  The *getaddrinfo* function returns the structures of the results. It is an array of pointers to struct addrinfo in which there are all the possible results returned by the DNS.
  Generally, a for cycle is used in order to check which is the struct which allows a connection.

  #### Examples
  client side:
  ``````
1 #include "unp.h"
2 int tcp_connect (const char *host, const char *serv)
3 {
4
5 int sockfd, n;
6 struct addrinfo hints, *res, *ressave;

7 bzero(&hints, sizeof (struct addrinfo));
8 hints.ai_family = AF_UNSPEC;
9 hints.ai_socktype = SOCK_STREAM;
10 if ( (n = getaddrinfo (host, serv, &hints, &res)) != 0)
11    err_quit("tcp_connect error for %s, %s: %s",
12 host, serv, gai_strerror (n));
13 ressave = res;
14 do {
15    sockfd = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
16     if (sockfd < 0)
17        continue; /*ignore this one */
18     if (connect (sockfd, res->ai_addr, res->ai_addrlen) == 0)
19       break; /* success */
20     Close(sockfd); /* ignore this one */
21  } while ( (res = res->ai_next) != NULL);

22    if (res == NULL) /* errno set from final connect() */
23      err_sys ("tcp_connect error for %s, %s", host, serv);
24    freeaddrinfo (ressave);
25    return (sockfd);
26 }

  ``````

  server side:
  ``````
  1 #include "unp.h"
2 int
3 tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
4 {
5 int listenfd, n;
6 const int on = 1;
7 struct addrinfo hints, *res, *ressave;
8 bzero(&hints, sizeof (struct addrinfo)) ;
9 hints.ai_flags = AI_PASSIVE;
10 hints.ai_family = AF_UNSPEC;
11 hints.ai_socktype = SOCK_STREAM;
12  if ( (n = getaddrinfo (host, serv, &hints, &res)) != 0)
13    err_quit("tcp_listen error for %s, %s: %s",
14    host, serv, gai_strerror(n)) ;
15  ressave = res;
16  do {
17    listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
18
19    if (listenfd < 0)
20      continue; /* error, try next one */
21    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on) ) ;
22    if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
23      break; /* success */
24    Close (listenfd); /* bind error, close and try next one */
25  } while ( (res = res->ai_next) != NULL);

26  if (res == NULL) /* errno from final socket () or bind () */
27    err_sys ("tcp_listen error for %s, %s", host, serv);
28  Listen (listenfd, LISTENQ);
29  if (addrlenp)
30    *addrlenp = res->ai_addrlen; /* return size of protocol address */
31  freeaddrinfo (ressave);
32  return (listenfd);
33 }
  ``````

#### *getnameinfo* function
This function is the complement of getaddrinfo. It takes a structure and returns two strings: the host and service.  
Prototype:
``````
int getnameinfo(const struct sockaddr *sockaddr, socklen_t addrlen, char *host, socklen_t hostlen, char* serv, socklen_t servlen, int flags);

``````
The arguments are:
* *sockaddr* points to the socket address structure to be converted to string.
* *addrlen* is the len of this structure.
* *host* and *hostlen* specify the host string returned.
* *serv* and *servlen* specify the service to be returned.

**N.B:** Remember that if we are interested in the string format of the dotted decimal address, we have to use *__sock\_ntop__* for both IPv4 and IPv6.

``````
char *sock_ntop(const struct sockaddr *sockaddr, socklen_t addrlen);
``````
