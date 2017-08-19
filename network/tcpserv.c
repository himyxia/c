#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<strings.h>
#include<string.h>
#include<unistd.h>

int main(int argc, char *argv[]) {
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1) {
		fputs("socker error", stderr);
		return 1;
	}


	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8080);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int r;
	r = bind(listenfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
	if (r == -1) {
		fputs("bind error", stderr);
		return 1;
	}

	r = listen(listenfd, 5);
	if (r == -1) {
		fputs("listen error", stderr);
		return 1;
	}

	int connfd;
	int childpid;
	struct sockaddr_in cliaddr;
	bzero(&cliaddr, sizeof(cliaddr));

	puts("accept success1");
	for(; ;) {
		puts("accept success2");
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)sizeof(cliaddr));
		if (r == -1) {
			fputs("accept error3", stderr);
			return 1;
		}
		puts("accept success4");
		printf("cli:%d\n", cliaddr.sin_port);
		puts("accept success5");

		if((childpid = fork()) == 0) {
			close(listenfd);
			char buf[1024];
			while(1) {
				r = read(connfd, buf, 1024);
				if (r == -1) {
					return 1;
				}
				printf("receive %s\n", buf);
				write(connfd, buf, 1024);
			}
		}
		close(connfd);
	}
	return 0;
}
