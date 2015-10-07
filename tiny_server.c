#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXBUF 1000
#define MAXLINE 4096
#define RIO_BUFSIZE 8192

typedef struct {
    int rio_fd;                /* file descriptor */
    size_t rio_cnt;            /* number of unread bytes in rio_buf */
    char *rio_bufptr;          /* next unread byte in rio_buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
}rio_t;

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    if (n == 0)
        return 0;

    while (rp->rio_cnt == 0) { /* refill if buffer is empty */
    	ssize_t rc = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
   	    if (rc < 0) {            /* read() error */
        	if (errno == EINTR)    /* interrupted by a signal */
                continue;            /* no data was read, try again */
     		else
        		return -1;           /* errno set by read(), give up */
    	}
    if (rc == 0)             /* EOF */
        return 0;
    rp->rio_bufptr = rp->rio_buf;  /* read() success, buffer is filled */
    rp->rio_cnt = rc;        /* 0 < rc <= sizeof(rp->rio_buf) */
	}

  /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    size_t cnt = rp->rio_cnt;  /* 0 < rp->rio_cnt */
    if (n < cnt)               /* 0 < n */
        cnt = n;
    (void) memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;

    return cnt;
}

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;

    return;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    size_t n;
    char *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
    	ssize_t rc = rio_read(rp, bufp, 1);
    if (rc < 0)
        return -1;          /* errno set by read(), give up */
    if (rc == 0) {
        if (n == 1)
        	return 0;         /* EOF, no data read */
    	else
        	break;            /* EOF, some data was read */
    }
    if (*bufp++ == '\n')  /* read() success, 0 < rc <= 1 */
        break;
 	}
  	*bufp = '\0';

  	return n;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;        /* 0 <= nleft == n */
    char *bufp = usrbuf;

    while (nleft > 0) {      /* loop invariant: 0 <= nleft <= n */
        ssize_t rc = write(fd, bufp, nleft);
    	if (rc < 0) {          /* write() error */
        	if (errno == EINTR)  /* interrupted by a signal */
        		continue;          /* no data was written, try again */
        	else
        		return -1;         /* errno set by write(), give up */
   		}
    	if (rc == 0)           /* nothing written, but not an error */
       		continue;            /* try again */
    	bufp += rc;            /* write() success, 0 < rc <= nleft */
    	nleft -= rc;           /* 0 <= new nleft < old nleft <= n */
    }
  	return n;
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXBUF];

	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	rio_writen(fd, buf, strlen(buf));
	rio_writen(fd, body, strlen(body));
}

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
char *shortmsg, char *longmsg);

void doit(int fd)
{
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	rio_readinitb(&rio, fd);
	rio_readlineb(&rio, buf, MAXLINE);
	sscanf(buf, "%s %s %s", method, uri, version);
	if (strcasecmp(method, "GET")) {
		clienterror(fd, method, "501", "Not Implemented",
		"Tiny does not implement this method");
		return;
	}
	read_requesthdrs(&rio);

	is_static = parse_uri(uri, filename, cgiargs);
	if (stat(filename, &sbuf) < 0) {
		clienterror(fd, filename, "404", "Not found",
		"Tiny couldn’t find this file");
	return;
	}

	if (is_static) { 
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			clienterror(fd, filename, "403", "Forbidden", "Tiny couldn’t read the file");
		return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
			clienterror(fd, filename, "403", "Forbidden",
			"Tiny couldn’t run the CGI program");
			return;
		}
	serve_dynamic(fd, filename, cgiargs);
	}
}


void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];

	rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n")) {
		rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
	return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;

	if (!strstr(uri, "cgi-bin")) {
		strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		// if (uri[strlen(uri)-1] == "/")
		if (strcmp(&uri[strlen(uri)-1], "/") == 0)
			strcat(filename, "home.html");
		return 1;
	}
	else {
		ptr = index(uri, 1);
		if (ptr) {
			strcpy(cgiargs, ptr+1);
			*ptr = 0;
		}
		else
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

void serve_static(int fd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];

	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	rio_writen(fd, buf, strlen(buf));

	srcfd = open(filename, O_RDONLY, 0);
	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	rio_writen(fd, srcp, filesize);
	munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype)
{
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
}

extern char** environ;
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
	char buf[MAXLINE], *emptylist[] = { NULL };

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	rio_writen(fd, buf, strlen(buf));

	if (fork() == 0) {
		setenv("QUERY_STRING", cgiargs, 1);
		dup2(fd, STDOUT_FILENO);
		execve(filename, emptylist, environ);
	}
	wait(NULL);
}

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
   listen(listenfd, 1024);

   return listenfd;
}

int main(int argc, char **argv)
{
	typedef unsigned int socklen_t;
	socklen_t clientlen;
	int listenfd, connfd, port;
	struct sockaddr_in clientaddr;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);

	listenfd = open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		doit(connfd);
		close(connfd);
	}
}
