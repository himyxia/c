#include "net.h"

static char read_buf[MAXLINE];
static char *read_ptr;
static int read_cnt;

/*
 * my_read() read MAXLINE size bytes in to buf
 * and afterwards each call to my_read() just
 * fetch sigle byte from the buf
*/
static ssize_t my_read(int fd, char *ptr)
{
	if(read_cnt <= 0) {
again:
		if((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
				goto again;
			return -1;
		}else if (read_cnt == 0) 
			return 0; // EOF

		read_ptr = read_buf;
	}


	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
	ssize_t i, rc;
	char c, *ptr;

	ptr = vptr;
	for (i = 0; i < maxlen; i++) {
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		}else if (rc == 0) {
			// eof
			*ptr = 0;
			return (i - 1);
		} else 
			return (-1);
	}
	*ptr = 0;
	return i;
}

ssize_t Readline(int fd, void *ptr, size_t maxlen) 
{
	ssize_t n;
	if((n = readline(fd, ptr, maxlen)) < 0) 
		err_sys("readline error");
	return n;
}
