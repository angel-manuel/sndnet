#include "sndnet.h"

#include <assert.h>
#include <arpa/inet.h>
#include <nacl/crypto_box.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct SNMessage_ {
	unsigned char dst[crypto_box_PUBLICKEYBYTES];
	unsigned char src[crypto_box_PUBLICKEYBYTES];
	unsigned short ttl;
	unsigned int len;
} SNMessage;

void sndnet_log(SNState* sns, const char* format, ...);
void* sndnet_background(void* arg);

void default_log_cb(const char* msg) {
	fprintf(stderr, "sndnet: %s\n", msg);
}

int sndnet_init(SNState* sns, unsigned short port) {
	int socket_fd;
	struct sockaddr_in serv_addr;
	
	assert(sns != 0);
	
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
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	
	if(bind(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
		sndnet_log(sns, "Error while initializing socket");
		close(socket_fd);
		return 1;
	}
	
	sns->socket_fd = socket_fd;
	
	/* Background thread initialization */
	
	if(pthread_create(&(sns->bg_thrd), 0, sndnet_background, sns)) {
		sndnet_log(sns, "Error while starting thread");
		close(socket_fd);
		return 1;
	}
	
	sndnet_log(sns, "Initialized");
	
	return 0;
}

void sndnet_destroy(SNState* sns) {
	assert(sns != 0);
	
	sndnet_log(sns, "Destroying");
	
	/* Socket closing */
	
	close(sns->socket_fd);
	
	/* Thread closing */
	
	pthread_cancel(sns->bg_thrd);
	pthread_join(sns->bg_thrd, 0);
	
	sndnet_log(sns, "Destroyed");
}

void sndnet_set_log_callback(SNState* sns, sndnet_log_callback cb) {
	assert(sns != 0);
	
	if(cb)
		sns->log_cb = cb;
	else
		sns->log_cb = default_log_cb;
	
	sndnet_log(sns, "Log callback changed");
}

void sndnet_log(SNState* sns, const char* format, ...) {
	char str[1024];
	va_list args;
	
	assert(sns != 0);
	assert(sns->log_cb != 0);
	assert(format != 0);
	
	va_start(args, format);
	vsnprintf(str, 1024, format, args);
	va_end(args);
	
	sns->log_cb(str);
}

void* sndnet_background(void* arg) {
	SNState* sns = (SNState*)arg;
	SNMessage msg;
	struct sockaddr rem_addr;
	socklen_t addrlen = sizeof(rem_addr);
	int recv_count;
	
	assert(sns != 0);
	
	do {
		memset(&msg, 0, sizeof(msg));
		
		recv_count = recvfrom(sns->socket_fd, &msg, sizeof(msg), 0, &rem_addr, &addrlen);
		
		/*if(strncmp(msg.dst, "quit", 4) == 0) {
			return 0;
		}*/
		
		sndnet_log(sns, "msg\n"
		"dst = %.32s\n"
		"src = %.32s\n"
		"ttl = %hu\n"
		"len = %u\n",
		msg.dst, msg.src, msg.ttl, msg.len);
	} while(recv_count > 0);
	
	return 0;
}
