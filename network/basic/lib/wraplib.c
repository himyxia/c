#include "net.h"

ssize_t writen(int fd, const void *vptr, size_t n) 
{
	ssize_t nwritten;
	const char *ptr;
	size_t nleft;

	ptr = vptr;
	nleft = n;
	while(nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR) 
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

void Writen(int fd, void *ptr, size_t nbytes)
{
	if(writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}

void Close(int fd)
{
	/* what happens here? */
	if(close(fd) == -1)
		err_sys("close error");
}

pid_t Fork(void) 
{
	pid_t pid;
	if((pid = fork()) == -1)
		err_sys("fork error");
	return pid;
}

void
Inet_pton(int family, const char *strptr, void *addrptr) {
	int n;
	if( (n = inet_pton(family, strptr, addrptr) ) < 0) 
		err_sys("inet_pton error for %s", strptr); /* errno set */
	else if (n == 0)
		err_quit("inet_pton error for %s", strptr); /* errno not set */
}
