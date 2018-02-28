#include <stdio.h>

// writer
int main(int argc, char **argv) {
	int fd;
	char *myfifo = "/tmp/myfifo";

	mkfifo(myfifo, 0666);

	fd = open(myfifo, O_WRONLY);
	write(fd, "Hi", sizeof("Hi"));
	close(fd);

	unlink(myfifo);

	return 0;
}

// reader
int main(int argc, char **argv) {
	int fd;
	char *myfifo = "/tmp/myfifo";
	char buf[MAX_BUF];

	fd = open(myfifo, O_RDONLY);
	read(fd, buf, MAX_BUF);
	printf("Received: %s\n", buf);
	close(fd);

	return 0;
}
