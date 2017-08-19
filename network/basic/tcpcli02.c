#include "lib/net.h"

int main(int argc, char* argv[])
{
	int socketfd; 
	struct sockaddr_in servaddr;

	if(argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	socketfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	Connect(socketfd, (SA*)&servaddr, sizeof(servaddr));

	//str_cli(stdin, socketfd);

	char recvline[MAXLINE], sendline[MAXLINE];
	while(Fgets(sendline, MAXLINE, stdin) != NULL) {
		Writen(socketfd, sendline, 1);
		sleep(1);
		Writen(socketfd, sendline+1, strlen(sendline) - 1);

		if (Readline(socketfd, recvline, MAXLINE) == 0)
			err_quit("server terminated prematurely");
		Fputs(recvline, stdout);
	}
	return 0;
}
