#include "csapp.h"

void doit(int fd);
void read_requesthdrs(int fd);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char cgiargs);
void clienterror(int fd, char *cause, char *errnum,
		char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;

	if (argc != 2) {
		fprintf(stderr, "usgage: %s <port>\n", argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);

	listenfd = open_listenfd(port);
	while(1) {
		clientlen = sizeof(chlientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		doit(connfd);
		Close(connfd);
	}
}

void doit(int fd) {
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
	char filename[MAXLINE],cgiargs[MAXLINE];

	Readline(fd, buf, MAXLINE);
	sscanf(buf, "%s %s %s\n", method, uri, version);
	if(strcasecmp(method, "GET")) {
		clienterror(fd, method, "501", "Not Implemented", 
				"Tiny does not implement this method");
		return;
	}
	read_requesthdrs(fd);

	is_static = parse_uri(uri, filename, cgiargs);
	if(stat(filename, &sbuf) < 0) {
		clienterror(fd, filename, "404", "Not found",
				"Tiny couldn't find this file");
		return;
	}

	if(is_static) {
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			clienterror(fd, filename, "403", "Forbidden",
					"Tiny couldn't read the file");
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}else {
		if(!(S_ISREG(sbug.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
			clienterror(fd, filename, "403", "Forbidden",
					"Tiny could't run the CGI program");
			return;
		}
		serve_dynamic(fd, filename, cgiargs);
	}
}

void clienterror(int fd, char *cause, char *errnum,
		char *shortmsg, char *longmsg) {
	char buf[MAXLINE], body[MAXBUF];

	/* build the http response body */
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	/* print the http response */
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", strlen(body));
	Writen(fd, buf, strlen(buf));
	Writen(fd, body, strlen(body));
}

void read_requesthdrs(int fd) {
	char buf[MAXLINE];

	Readline(fd, buf, MAXLINE);
	while(strcmp(buf, "\r\n"))
		Readline(fd, buf, MAXLINE);
	return;
}


int parse_uri(char *uri, char *filename, char *cgiargs) {
	char *ptr;

	if(!strstr(uri, "cgi-bin")) {
		strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		if(uri[strlen(uri) - 1] == '/') 
			strcat(filename, "home.html");
		return 1;
	} else {
		ptr = index(uri, '?');
		if(ptr) {
			strcpy(cgiargs, ptr+1);
			*ptr = '\0';
		}else 
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

void serve_static(int fd, char *filename, int filesize) {
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];

	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	Writen(fd, buf, strlen(buf));

	srcfd = Open(filename, O_RDONLY, 0);
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);
	Writen(fd, srcp, filesize);
	Munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype) {
	if(strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if(strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if(strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpg");
	else
		strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
	char buf[MAXLINE];

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Writen(fd, buf, strlen(buf));

	if(Fork() == 0) {
		setenv("QUERY_STRING", cgiargs, 1);
		Dup2(fd, STDOUT_FILENO);
		Execve(filename, NULL, environ);
	}
	Wait(NULL);
}
