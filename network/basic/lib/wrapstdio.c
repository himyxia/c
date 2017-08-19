#include "net.h"

void Fputs(const char *ptr, FILE *f) 
{
	if (fputs(ptr, f) == EOF) {
		err_sys("fputs error");
	}
}

char *Fgets(char *ptr, int n, FILE *stream)
{
	char *rptr;
	if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream)) {
		err_sys("fgets error");
	}
	return rptr;
}

