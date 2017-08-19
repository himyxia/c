#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<strings.h>
#include<unistd.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fputs("fuck you", stderr);
		return 1;
	}
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = 8080;
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	bzero(&servaddr, sizeof(servaddr));

	int connfd = connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if(connfd != 0) {
		puts("connect error");
		return 1;
	}

	char buf[1024];
	char recv[1024];
	while(fgets(buf, 1024,stdin) != NULL) {
		if (write(connfd, buf, 1024)==-1) {
			puts("write error");
			return 1;
		}

		if (read(connfd, recv, 1024) == -1) {
			puts("read error");
			return 1;
		}

		printf("%s", recv);
	}

	close(connfd);

	return 0;
}
