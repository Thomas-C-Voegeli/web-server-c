#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#define LISTENQ 1024

int open_listenfd(int port) 
{
   int listenfd, optval=1;
   struct sockaddr_in serveraddr;
  
   /* Create a socket descriptor */
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      return -1;
   }
 
   /* Eliminates "Address already in use" error from bind. */
   if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
      return -1;
   }
   /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
   memset((char *) &serveraddr, 0, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET; 
   serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
   serveraddr.sin_port = htons((unsigned short)port); 
   
   // if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1){
      // return -1;
   // }
   bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
   listen(listenfd, LISTENQ);
    /* Make it a listening socket ready to accept connection requests */
   // if (listen(listenfd, LISTENQ) == -1){
      // return -1;
   // }

   return listenfd;
}

void client_request(int connfd)
{
   write(connfd, "HTTP/1.1 200 OK\n", 16);
   write(connfd, "Content-length: 70\n", 19);
   write(connfd, "Content-Type: text/html\n\n", 25);
   write(connfd, "<html><body><H1>Welcome to Thomas' homemade server!</H1></body></html>",70);
}

char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
// "<style>body { background-color: #111 }"
// "h1 { font-size:4cm; text-align: center; color: black;"
// "text-shadow: 0 0 2mm red</style></head>"
"<body><h1>Monogomous Server</h1></body></html>\r\n";

int main(int argc, char **argv)
{
   typedef unsigned int socklen_t;
   int listenfd, connfd, port;
   socklen_t clientlen;
   struct sockaddr_in clientaddr;
   struct hostent *host_p;
   char *haddrp;
   
   if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      exit(0);
   }

   port = atoi(argv[1]);
   listenfd = open_listenfd(port);

   while (1) {
      clientlen = sizeof(clientaddr);
      connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
      host_p = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      haddrp = inet_ntoa(clientaddr.sin_addr);
      printf("server connected to %s (%s)\n", host_p->h_name, haddrp);
      // write(connfd, "HTTP/1.1 200 OK\n", 16);
      // write(connfd, "Content-length: 70\n", 19);
      // write(connfd, "Content-Type: text/html\n\n", 25);
      // write(connfd, "<html><body><H1>Welcome to Thomas' homemade server!</H1></body></html>",70);
      // echo(connfd);
      write(connfd, response, sizeof(response) - 1);
      close(connfd);
   }
   exit(0);
}