#ifndef _net_h
#define _net_h

#define MAXLINE 100
#define FD_SETSIZE 100

#define max(a,b) ((a) > (b) ? (a):(b))
#define min(a,b) ((a) < (b) ? (a):(b))

#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>


#define SERV_PORT 3034
#define LISTENQ 1024
/* generic sockaddr */
#define SA struct sockaddr

void err_quit(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void Writen(int fd, void *ptr, size_t nbytes);

pid_t Fork(void);

void Close(int fd);
int Accept(int fd, struct sockaddr *sa, socklen_t *l);
void Listen(int fd, int backlog);
void Bind(int fd, const struct sockaddr *sa, socklen_t l);
int Socket(int family, int socket_type, int protocol);

ssize_t Readline(int fd, void *ptr, size_t maxlen);

char *Fgets(char *ptr, int n, FILE *stream);
void Fputs(const char *ptr, FILE *f);

// Sigfunc is a function type: void (int)
// *Sigfunc is a pointer to any function: void (int)
typedef void Sigfunc(int);

Sigfunc* Signal(int signo, Sigfunc *func);

void str_cli(FILE *fd, int sockfd);

void str_echo(int connfd);

void Inet_pton(int family, const char *strptr, void *addrptr);
void Connect(int sockfd, struct sockaddr *a, socklen_t t);

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

void Shutdown(int fd, int how);

ssize_t Read(int fd, void *ptr, size_t nbytes);
#endif
