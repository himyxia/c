#include "net.h"

void
str_cli(FILE *fp, int sockfd)
{
	char recvline[MAXLINE], sendline[MAXLINE];

	while(Fgets(sendline, MAXLINE, fp) != NULL) {
		Writen(sockfd, sendline, strlen(sendline));
		if (Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("server terminated prematurely");
		Fputs(recvline, stdout);
	}
}
