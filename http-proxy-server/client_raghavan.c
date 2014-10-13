/* 
ECEN602: Computer Networks
HW3 programming assignment: HTTP1.0 client GET request
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define MAXDATASIZE 512 // max number of bytes we can get at once 


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char const *argv[])
{
	/* write HTTP1.0 client code */
	/* example of GET request for www.google.com

	    char query[] =
        "GET / HTTP/1.0\r\n"
        "Host: www.google.com\r\n"
        "\r\n";

    */

	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 4) {
	fprintf(stderr,"usage: client <proxy address> <proxy port> <url to retrieve>\n");
	exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) {
	    perror("client: socket");
	    continue;
	}

	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    close(sockfd);
	    perror("client: connect");
	    continue;
	}

	break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	//Obtaining the IPv4/IPv6 addresses in text format
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	    s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	// FD_SET tmp variable for select() 
	fd_set tmp;

	while(1)
	{
	    FD_ZERO(&tmp);
	    FD_SET(0,&tmp);
	    FD_SET(sockfd,&tmp);

	    if(select(sockfd+1,&tmp,NULL,NULL,NULL) == -1) {
		printf("Error with select \n");
		perror("select");
		exit(1);
	    }
	    
	    if(FD_ISSET(0,&tmp))
	    {
		char query[MAXDATASIZE] = { }; /* = "GET / HTTP/1.0\r\n" "Host: www.google.com\r\n" "\r\n"*/
		sprintf(query, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n",argv[3]);		
		if (send(sockfd, query, sizeof(query), 0) == -1){
		printf("Error sending\n");
		perror("send");
	    	}
	    }
	    
	    if(FD_ISSET(sockfd,&tmp))
	    {
		if ((numbytes = recv(sockfd, buf,sizeof(buf), 0)) == -1) {
		    perror("recv");
	     	       exit(1);
		}

		buf[numbytes] = '\0';

		printf("client: received '%s'\n",buf);
	    }
	}
	
	close(sockfd);


	return 0;
}