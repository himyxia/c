#include "lib/net.h"

int main(int argc, char **argv) 
{
	fd_set rset, allset;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	int nready, client[FD_SETSIZE];
	int sockfd, connfd, maxfd, maxi, listenfd;
	int i,n, buf[MAXLINE];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	maxfd = listenfd;
	// max fd index
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for(; ;) {
		rset = allset;

		// waits for something to happen: either the establishment of a new client
		// connection or the arrival of data, a FIN, or an RST on an existing connection
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

		// after calling select, any fd which are not ready will be cleared
		// so if linstenfd is set after that call, only means it is ready
		if(FD_ISSET(listenfd, &rset)) {
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);

			for(i = 0; i < FD_SETSIZE; i++) {
				if(client[i] < 0) {
					client[i] = connfd;
					break;
				}
			}
			if (i == FD_SETSIZE) {
					err_quit("too many clients");
			}
			FD_SET(connfd, &allset);
			if(connfd > maxfd) {
				maxfd = connfd;
			}
			if (i > maxi) {
				maxi = i;
			}
			if(--nready <= 0) {
				// timeout or err occur
				continue;
			}
		}

		for(i = 0; i <= maxi; i++) {
			if((sockfd = client[i]) < 0) {
				continue;
			}
			if(FD_ISSET(sockfd, &rset)) {
				if((n = Read(sockfd, buf, MAXLINE)) == 0 ) {
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}else {
					Writen(sockfd, buf, n);
				}

				if(--nready <= 0) 
					break;
			}
		}
	}
}
