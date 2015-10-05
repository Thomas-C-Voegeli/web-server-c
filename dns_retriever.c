#include "stdlib.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
// #define _XOPEN_SOURCE 500

int main(int argc, char **argv)
{
	// getaddrinfo(const char *node, const char *service,
 //                       const struct addrinfo *hints,
                       // struct addrinfo **res);

	// char *hostname = 
	// struct hostent {
	// 	char *h_name; /* Official domain name of host */
	// 	char **h_aliases; /* Null-terminated array of domain names */
	// 	int h_addrtype;  Host address type (AF_INET) 
	// 	int h_length; /* Length of an address, in bytes */
	// 	char **h_addr_list; /* Null-terminated array of in_addr structs */
	// };

	char **pp;
	struct in_addr addr;
	struct hostent *hostp;
	// struct addrinfo hints, *res;
	///Test area

	// memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;
	// hints.ai_family = AF_UNSPEC;
	// struct addrinfo test_address;
	// getaddrinfo(argv[1], 0, &hints, &res);
	// char test = 't';
	// char *test_var = &test;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <domain name or dotted-decimal>\n", argv[0]);
		exit(0);
	}

	if (inet_aton(argv[1], &addr) != 0){
		hostp = gethostbyaddr((const char *)&addr, sizeof(addr), AF_INET);	
	}
	else{
		hostp = gethostbyname(argv[1]);
	}


	// if(getaddrinfo(hostp->h_name, 0, &hints, &res) != 0){
	// 	const char *error_message;
	// 	error_message = gai_strerror(error_num);
	// 	printf("error: %s\n", error_message);
	// 	exit(0);
	// }

	printf("official hostname: %s\n", hostp->h_name);

	for (pp = hostp->h_aliases; *pp != NULL; pp++){
		printf("alias: %s\n", *pp);
	}

	for (pp = hostp->h_addr_list; *pp != NULL; pp++) {
		addr.s_addr = ((struct in_addr *)*pp)->s_addr;
		printf("address: %s\n", inet_ntoa(addr));
	}
	exit(0);
}