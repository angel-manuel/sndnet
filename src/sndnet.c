#include "sndnet.h"

#include <assert.h>
#include <stdio.h>
#include <tinycthread.h>

void sndnet_log(SNState* sns, const char* msg);

void default_log_cb(const char* msg) {
	fprintf(stderr, "sndnet: %s\n", msg);
}

int sndnet_init(SNState* sns, short port) {
	assert(sns != NULL);
	
	sns->log_cb = default_log_cb;
	sns->port = port;
	
	sndnet_log(sns, "Initialized");
	
	return 0;
}

void snd_destroy(SNState* sns) {
	sndnet_log(sns, "Destroying");
	assert(sns != NULL);
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
