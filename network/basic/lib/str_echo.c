#include "net.h"

void str_echo(int connfd)
{
	ssize_t n;
	char buf[MAXLINE];

again:
	while ((n = read(connfd, buf, MAXLINE)) > 0) {
		Fputs(buf, stdout);
		Writen(connfd, buf, n);
	}

	if (n < 0 && errno == EINTR)
		goto again;
	else if(n < 0)
		err_sys("str_echo: read error");
}

