#include "net.h"

void
Shutdown(int fd, int how)
{
	if (shutdown(fd, how) < 0)
		err_sys("shutdown error");
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int n;

	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
		err_sys("select error");
	return (n);
}

void
Connect(int sockfd, struct sockaddr *a, socklen_t t)
{
	if (connect(sockfd, a, t) < 0) {
		err_sys("connect error");
	}
}

int Accept(int fd, struct sockaddr *sa, socklen_t *l) 
{
	int n;

again:
	if((n = accept(fd, sa, l)) < 0) {
#ifdef ERROTO
		if(errno == ERROTO || errno == ECONNABORTED)
#else
		if(errno == ECONNABORTED)
#endif
			goto again;
	else
		err_sys("accept error");

	}
	return n;
}

void Listen(int fd, int backlog) 
{
	char *ptr;
	if((ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if(listen(fd, backlog) < 0)
		err_sys("listen error");
}

void Bind(int fd, const struct sockaddr *sa, socklen_t l) 
{
	if(bind(fd, sa, l) < 0)
		err_sys("bind error");
}

int Socket(int family, int socket_type, int protocol)
{
	int socketfd;

	if ( (socketfd = socket(family, socket_type, protocol)) < 0)
		err_quit("err create socket");

	return socketfd;
}

