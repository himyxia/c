CC = gcc
CFLAGS = -g -O2 -D_REETRANT -Wall
LIBS = libnet.a -lpthread
#../unpv13e/libunp.a 

PROGS = ex1 tcpcli01 tcpcli02 tcpcli04  tcpserv01 tcpserv02 tcpserv03 tcpservselect01

OBJFILES = ex1.o tcpcli01.o tcpcli02.o tcpserv03.o tcpcli04.o tcpserv01.o tcpserv02.o sigchldwait.o sigchldwaitpid.o strcliselect01.o tcpservselect01.o

all: $(PROGS)

ex1: ex1.o
	$(CC) $(CFLAGS) -o $@ ex1.o $(LIBS)

tcpcli02: tcpcli02.o
	$(CC) $(CFLAGS) -o $@ tcpcli02.o $(LIBS)

tcpcli01: tcpcli01.o strcliselect01.o
	$(CC) $(CFLAGS) -o $@ tcpcli01.o strcliselect01.o $(LIBS)

tcpcli04: tcpcli04.o
	$(CC) $(CFLAGS) -o $@ tcpcli04.o $(LIBS)

tcpserv01: tcpserv01.o 
	$(CC) $(CFLAGS) -o $@ tcpserv01.o $(LIBS)

tcpserv02: tcpserv02.o sigchldwait.o
	$(CC) $(CFLAGS) -o $@ tcpserv02.o sigchldwait.o $(LIBS)

tcpserv03: tcpserv03.o sigchldwaitpid.o
	$(CC) $(CFLAGS) -o $@ tcpserv03.o sigchldwaitpid.o $(LIBS)

tcpservselect01: tcpservselect01.o sigchldwaitpid.o
	$(CC) $(CFLAGS) -o $@ tcpservselect01.o sigchldwaitpid.o $(LIBS)

clean:
	rm $(PROGS) $(OBJFILES)
