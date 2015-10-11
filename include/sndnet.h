/**
 * @file
 * SecondNet main header
 * */


#ifndef SN_SN_H_
#define SN_SN_H_

#include "addr.h"
#include "msg.h"
#include "sock.h"
#include "router.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Holds the state of a node.
 * Should NOT be modified directly.
 * */
typedef struct sn_state_t_ sn_state_t;

/**
 * Callback for logging
 * */
typedef void (*sn_log_callback_t)(const char* msg, void* extra);

/**
 * Callback for forward
 * */
typedef void (*sn_forward_callback_t)(const sn_msg_t* msg, sn_state_t* sns, sn_entry_t* nexthop, void* extra);

/**
 * Callback for delivery
 * */
typedef void (*sn_deliver_callback_t)(const sn_msg_t* msg, sn_state_t* sns, void* extra);

/**
 * Initialization of a node
 * @param sns State to be initialized(must be already allocated)
 * @param self Node address
 * @param socket Listening socket
 * @return 0 if OK, -1 otherwise
 * */
int sn_init(sn_state_t* sns, const sn_addr_t* self, const sn_sock_t* socket);

/**
 * Destroys a node
 * @param sns State to be destroyed(but not deallocated)
 * */
void sn_destroy(sn_state_t* sns);

/**
 * Changes the logging callback
 * @param sns Node state
 * @param[in] cb The new callback. If it is NULL, default logging(stderr) will be used
 * */
void sn_set_log_callback(sn_state_t* sns, sn_log_callback_t cb, void* extra);

/**
 * Changes the forwarding callback
 * @param sns Node state
 * @param[in] cb The new callback. If it is NULL default forwarding will be used.
 * */
void sn_set_forward_callback(sn_state_t* sns, sn_forward_callback_t cb, void* extra);

/**
 * Changes the delivering callback
 * @param sns Node state
 * @param[in] cb The new callback. If it is NULL, message will be just logged.
 * */
void sn_set_deliver_callback(sn_state_t* sns, sn_deliver_callback_t cb, void* extra);

/**
 * Sends a message without need for acknowledgement
 * @param sns Node state
 * @param dst Destination address
 * @param len Message length
 * @param payload Message payload
 * @return -1 if error
 */
int sn_send(sn_state_t* sns, const sn_addr_t* dst, size_t len, const char* payload);

/**
 * Joins a SecondNet network using a know gateway
 * @param sns Node state
 * @param gateway Network address of the gateway
 * @return 0 if corretly joined, -1 otherwise
 * */
int sn_join(sn_state_t* sns, const sn_netaddr_t* gateway);

void sn_silent_log_callback(const char* msg, void* extra);
void sn_named_log_callback(const char* msg, void* extra);

struct sn_state_t_ {
    sn_addr_t self; /**< Node SecondNet address */
    sn_router_t router; /**< Routing state */
    pthread_t bg_thrd; /**< Background thread for routing */
    sn_log_callback_t log_cb; /**< Callback for logging */
    void* log_extra; /**< Extra data for log callback */
    sn_forward_callback_t forward_cb; /**< Callback for forward */
    void* forward_extra; /**< Extra data for forward callback */
    sn_deliver_callback_t deliver_cb; /**< Callback for deliver */
    void* deliver_extra; /**< Extra data for deliver callback */
    sn_sock_t socket; /**< Listening socket file descriptor */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_SN_H_*/
