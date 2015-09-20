/**
 * @file
 * SecondNet main header
 * */


#ifndef SNDNET_SNDNET_H_
#define SNDNET_SNDNET_H_

#include "address.h"
#include "message.h"
#include "router.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Holds the state of a node.
 * Should NOT be modified directly.
 * */
typedef struct SNState_ SNState;

/**
 * Callback for logging
 * */
typedef void (*sndnet_log_callback)(const char* msg);

/**
 * Callback for forward
 * */
typedef void (*sndnet_forward_callback)(const SNMessage* msg, SNState* sns, SNEntry* nexthop);

/**
 * Callback for delivery
 * */
typedef void (*sndnet_deliver_callback)(const SNMessage* msg, SNState* sns);

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

/**
 * Changes the forwarding callback
 * @param sns Node state
 * @param[in] cb The new callback. If it is NULL default forwarding will be used.
 * */
void sndnet_set_forward_callback(SNState* sns, sndnet_forward_callback cb);

/**
 * Changes the delivering callback
 * @param sns Node state
 * @param[in] cb The new callback. If it is NULL, message will be just logged.
 * */
void sndnet_set_deliver_callback(SNState* sns, sndnet_deliver_callback cb);

/**
 * Sends a message without need for acknowledgement
 * @param sns Node state
 * @param dst Destination address
 * @param len Message length
 * @param payload Message payload
 * @return -1 if error
 */
int sndnet_send(const SNState* sns, const SNAddress* dst, size_t len, const char* payload);

struct SNState_ {
    SNAddress self;
    SNRouter router;
    pthread_t bg_thrd; /**< Background thread for routing */
    sndnet_log_callback log_cb; /**< Callback for logging */
    sndnet_forward_callback forward_cb; /**< Callback for forward */
    sndnet_deliver_callback deliver_cb; /**< Callback for deliver */
    unsigned short port; /**< Listening port */
    int socket_fd; /**< Listening socket file descriptor */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_SNDNET_H_*/
