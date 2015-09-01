#ifndef SNDNET_SNDNET_H_
#define SNDNET_SNDNET_H_

/**
 * Callback for logging
 * */
typedef void (*sndnet_log_callback)(const char* msg);

typedef struct SNState_ {
	short port;
	sndnet_log_callback log_cb;
} SNState;

/**
 * Initialization of a node
 * @param sns State to be initialized(must be already allocated)
 * @param[in] port UDP listening port
 * @return 0 if OK, -1 otherwise
 * */
int sndnet_init(SNState* sns, short port);

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
