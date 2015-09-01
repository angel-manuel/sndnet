#ifndef SNDNET_SNDNET_H_
#define SNDNET_SNDNET_H_

#include <tinycthread.h>

/**
 * Callback for logging
 * */
typedef void (*sndnet_log_callback)(const char* msg);

typedef struct SNState_ {
	thrd_t bg_thrd;
	sndnet_log_callback log_cb;
	unsigned short port;
	int socket_fd;
} SNState;

/**
 * Initialization of a node
 * @param sns State to be initialized(must be already allocated)
 * @param[in] port UDP listening port
 * @return 0 if OK, -1 otherwise
 * */
int sndnet_init(SNState* sns, unsigned short port);

/**
 * Destroys a node
 * @param sns State to be destroyed(but not deallocated)
 * */
void sndnet_destroy(SNState* sns);

/**
 * Changes the logging callback
 * @param sns Node state
 * @param[in] cb The new callback. If it is NULL, default logging(stderr) will be used
 * */
void sndnet_set_log_callback(SNState* sns, sndnet_log_callback cb);

#endif/*SNDNET_SNDNET_H_*/
