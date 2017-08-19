#include "lib/net.h"


int main(int argc, char *argv[]) 
{
	struct sockaddr_in cliaddr, servaddr;
	int listenfd, connfd;
	socklen_t clilen;
	pid_t childpid;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
	Listen(listenfd, LISTENQ);

	//printf("after listen() listenfd:%d\n", listenfd);
	for(;;) {
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
		/*
		printf("client addr port: %d\n", cliaddr.sin_port);
		printf("client addr %p\n", &cliaddr);
		printf("client connect fd: %d\n", connfd);
		*/

		if ((childpid = Fork()) == 0) {
			//	printf("before close() listenfd:%d\n", listenfd);
			// the listen socket and connfd are shared between parent and child
			Close(listenfd);
			//printf("after close() listenfd:%d\n", listenfd);
			str_echo(connfd);
			exit(0);
			// child process end
		}
		Close(connfd);
	}
}

