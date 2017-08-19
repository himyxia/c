#include "net.h"

ssize_t
Read(int fd, void *ptr, size_t nbytes) 
{
	ssize_t n;

	if ((n = read(fd, ptr, nbytes) ) == -1) {
		err_sys("read error");
	}
	return n;
}
