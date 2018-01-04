#include <stdio.h>
#include <pthread.h>
#include "util.h"

typedef struct { int sockfd; } thread_config_t;

typedef enum {WAIT_FOR_MSG, IN_MSG} ProcessingState;

void serve_connection(int sockfd) {
	if(send(sockfd, "*", 1, 0) < 1) {
		perror_die("send");
	}

	ProcessingState state = WAIT_FOR_MSG;

	while (1) {
		uint8_t buf[1024];
		int len = recv(sockfd, buf, sizeof buf, 0);
		if(len < 0) {
			perror_die("recv");
		}else if(len == 0) {
			break;
		}

		for (int i = 0; i < len; ++i) {
			switch(state) {
			case WAIT_FOR_MSG:
				if(buf[i] == '^') {
					state = IN_MSG;
				}
				break;
			case IN_MSG:
				if(buf[i] == '$') {
					state = WAIT_FOR_MSG;
				}else {
					buf[i] += 1;
					printf("start send \n");
					if(send(sockfd, &buf[i], 1, 0) < 1) {
						perror("send error");
						close(sockfd);
						return;
					}
				}
				break;
			}
		}
	}
	close(sockfd);
}

void *server_thread(void *arg) {
	thread_config_t *config = (thread_config_t*)arg;
	int sockfd = config->sockfd;
	free(config);

	unsigned long id = (unsigned long)pthread_self();
	printf("Thread %lu created to handle connection with socket %d\n", id, sockfd);

	serve_connection(sockfd);
	printf("Thread %lu done\n", id);
	return 0;
}

int main(int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0);

	int portnum = 9090;
	if(argc >= 2) {
		portnum = atoi(argv[1]);
	}
	printf("Serving on port %d\n", portnum);
	fflush(stdout);

	int sockfd = listen_inet_socket(portnum);

	while(1) {
		struct sockaddr_in peer_addr;
		socklen_t peer_addr_len = sizeof(peer_addr);

		int newsockfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);
		if(newsockfd < 0) {
			perror_die("ERROR on accept");
		}

		report_peer_connected(&peer_addr, peer_addr_len);
		pthread_t the_thread;

		thread_config_t *config = (thread_config_t*)malloc(sizeof(*config));
		if(!config) {
			die("OOM");
		}
		config->sockfd = newsockfd;
		pthread_create(&the_thread, NULL, server_thread, config);
		pthread_detach(the_thread);
	}
	return 0;
}
