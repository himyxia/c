#include "lib/net.h"

void echoAfterReadSocket(int socketfd);

int main(int argc, char *argv[]) 
{
	struct sockaddr_in cliaddr, servaddr;
	int listenfd, connfd;
	socklen_t clilen;
	pid_t childpid;
	void sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);

	//printf("after listen() listenfd:%d\n", listenfd);
	for( ; ; ) {
		clilen = sizeof(cliaddr);
		
		if((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) < 0) {
			if (errno == EINTR) {
				//err_sys("Got accpet error :)");
				printf("should be here");
				continue;
			}else{
				err_sys("Got accpet error not EINTR:)");
			}
		}

		if ((childpid = Fork()) == 0) {
			/*do not understand*/
			Close(listenfd);
			//printf("after close() listenfd:%d\n", listenfd);
			str_echo(connfd);
			exit(0);
			//child process end
		}
		Close(connfd); // parent process close conn socket
	}
}

void echoAfterReadSocket(int socketfd)
{
	ssize_t n;
	char buf[MAXLINE];

again:
	while ((n = read(socketfd, buf, MAXLINE)) > 0) {
		//printf("read from client\n");
		//printf("print it to screen \n");
		Fputs(buf, stdout);
		char sendline[MAXLINE];
		if (Fgets(sendline, MAXLINE, stdin)!= NULL ) {
		//	printf("reply to client \n");
			Writen(socketfd, sendline, strlen(sendline));
		}
	}
	
	if(n < 0 && errno == EINTR)
		goto again;
	else if(n < 0)
		err_sys("readFromSocke error");
}

