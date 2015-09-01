#include "sndnet.h"

#include <assert.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <tinycthread.h>

void sndnet_log(SNState* sns, const char* msg);
int sndnet_background(void* arg);

void default_log_cb(const char* msg) {
	fprintf(stderr, "sndnet: %s\n", msg);
}

int sndnet_init(SNState* sns, unsigned short port) {
	int socket_fd;
	struct sockaddr_in servaddr;
	
	assert(sns != NULL);
	
	/* Copying */
	
	sns->log_cb = default_log_cb;
	sns->port = port;
	
	sndnet_log(sns, "Initializing");
	
	/* Socket initialization */
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(socket_fd == -1) {
		sndnet_log(sns, "Error while initializing socket");
		return 1;
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
	if(bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		sndnet_log(sns, "Error while initializing socket");
		close(socket_fd);
		return 1;
	}
	
	sns->socket_fd = socket_fd;
	
	/* Background thread initialization */
	
	if(thrd_create(&(sns->bg_thrd), sndnet_background, sns) != thrd_success) {
		sndnet_log(sns, "Error while starting thread");
		close(socket_fd);
		return 1;
	}
	
	sndnet_log(sns, "Initialized");
	
	return 0;
}

void sndnet_destroy(SNState* sns) {
	assert(sns != NULL);
	
	sndnet_log(sns, "Destroying");
	
	/* Socket closing */
	
	close(sns->socket_fd);
	
	/* Thread closing */
	
	sndnet_log(sns, "Destroyed");
}

void sndnet_set_log_callback(SNState* sns, sndnet_log_callback cb) {
	assert(sns != NULL);
	
	if(cb)
		sns->log_cb = cb;
	else
		sns->log_cb = default_log_cb;
	
	sndnet_log(sns, "Log callback changed");
}

void sndnet_log(SNState* sns, const char* msg) {
	assert(sns != NULL);
	assert(sns->log_cb != NULL);
	assert(msg != NULL);
	
	sns->log_cb(msg);
}

int sndnet_background(void* arg) {
	SNState* sns = (SNState*)arg;
	char buff[1024];
	struct sockaddr remaddr;
	socklen_t addrlen = sizeof(remaddr);
	int recv_count;
	
	assert(sns != NULL);
	
	do {
		memset(buff, 0, 1024);
		
		recv_count = recvfrom(sns->socket_fd, buff, 1024, 0, &remaddr, &addrlen);
		
		sndnet_log(sns, buff);
		
		if(strncasecmp(buff, "quit", 4) == 0) {
			break;
		}
	} while(recv_count > 0);
	
	return 0;
}
