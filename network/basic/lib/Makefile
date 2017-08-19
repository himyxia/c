LIBOBJS = error.o readline.o wraplib.o wrapstdio.o wrapsock.o signal.o str_cli.o str_echo.o wrapunix.o
CC = gcc
CFLAGS = -c -Wall -O2 

ARFILE = ../libnet.a

all: ${LIBOBJS}
	ar rvs $(ARFILE) $?

error.o:
	$(CC) $(CFLAGS) error.c

wrapsock.o:
	$(CC) $(CFLAGS) wrapsock.c

wrapstdio.o:
	$(CC) $(CFLAGS) wrapstdio.c

wraplib.o:
	$(CC) $(CFLAGS) wraplib.c

wrapunix.o:
	$(CC) $(CFLAGS) wrapunix.c

readline.o:
	$(CC) $(CFLAGS) readline.c

signal.o:
	$(CC) $(CFLAGS) signal.c

str_cli.o:
	$(CC) $(CFLAGS) str_cli.c

str_echo.o:
	$(CC) $(CFLAGS) str_echo.c

clean:
	rm $(LIBOBJS)
