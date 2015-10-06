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
   int listenfd, option_value=1;
   struct sockaddr_in serveraddr;
  
   listenfd = socket(AF_INET, SOCK_STREAM, 0);
   setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&option_value , sizeof(int));

   memset((char *) &serveraddr, 0, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET; 
   serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
   serveraddr.sin_port = htons((unsigned short)port); 

   bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
   listen(listenfd, LISTENQ);

   return listenfd;
}

void send_response(int connfd)
{
   write(connfd, "HTTP/1.1 200 OK\n", 16);
   write(connfd, "Content-length: 70\n", 19);
   write(connfd, "Content-Type: text/html\n\n", 25);
   write(connfd, "<html><body><h1>Welcome to Thomas' homemade server!</h1></body></html>",70);
}

char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
"<style>body { background-color: grey; }"
"h1 { font-size:40px; text-align: center; color: blue; }"
"</style></head>"
"<body><h1>Thomas' Monogomous Server</h1></body></html>\r\n";

int main(int argc, char **argv)
{
   typedef unsigned int socklen_t;
   int listenfd, connfd, port;
   socklen_t clientlen;
   struct sockaddr_in clientaddr;
   struct hostent *host_p;
   char *haddrp;
   
   if (argc != 2) {
      fprintf(stderr, "must define port number after %s\n", argv[0]);
      exit(0);
   }

   port = atoi(argv[1]);
   listenfd = open_listenfd(port);

   while (1) {
      clientlen = sizeof(clientaddr);
      connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
      host_p = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      haddrp = inet_ntoa(clientaddr.sin_addr);
      printf("Connected to %s (%s)\n", host_p->h_name, haddrp);
      // send_response(connfd);
      write(connfd, response, sizeof(response) - 1);
      close(connfd);
   }
   exit(0);
}