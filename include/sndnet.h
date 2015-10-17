/**
 * @file
 * SecondNet main header
 * */


#ifndef SN_SN_H_
#define SN_SN_H_

#include "addr.h"
#include "closure.h"
#include "packet.h"
#include "io/sock.h"
#include "router.h"

#include <pthread.h>

#define asm __asm
#include <mintomic/mintomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Holds the state of a node.
 * Should NOT be modified directly.
 * */
typedef struct sn_state_t_ sn_state_t;

/**
 * Initializes a node
 * @param sns State to be initialized(must be already allocated)
 * @param self Node address
 * @param socket Listening socket
 * @return 0 if OK, -1 otherwise
 * */
int sn_init(sn_state_t* sns, const sn_addr_t* self, const sn_io_sock_t socket);

/**
 * Initializes a node from a string address and a listening port
 * @param sns State to be initialized(must be already allocated)
 * @param hexaddr String representation of the node SecondNet address
 * @param port Listening port number.
 * @return 0 if OK, -1 otherwise
 * */
int sn_init_at_port(sn_state_t* sns, const char hexaddr[SN_ADDR_PRINTABLE_LEN], uint16_t port);

/**
 * Destroys a node
 * @param sns State to be destroyed(but not deallocated)
 * */
void sn_destroy(sn_state_t* sns);

/**
 * Changes the logging callback
 * @param sns Node state
 * @param cb The new callback. If it is NULL, default logging(stderr) will be used
 * */
void sn_set_log_callback(sn_state_t* sns, sn_closure_t* cb);

/**
 * Changes the forwarding callback
 * @param sns Node state
 * @param cb The new callback. If it is NULL default forwarding will be used.
 * */
void sn_set_forward_callback(sn_state_t* sns, sn_closure_t* cb);

/**
 * Changes the delivering callback
 * @param sns Node state
 * @param cb The new callback. If it is NULL, message will be just logged.
 * */
void sn_set_deliver_callback(sn_state_t* sns, sn_closure_t* cb);

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
int sn_join(sn_state_t* sns, const sn_io_naddr_t* gateway);

void sn_silent_log_callback(int argc, void* argv[]);
void sn_named_log_callback(int argc, void* argv[]);

struct sn_state_t_ {
    /* Background thread state */
    sn_addr_t self; /**< Node SecondNet address */
    sn_router_t router; /**< Routing state */
    pthread_t bg_thrd; /**< Background thread for routing */
    sn_io_sock_t socket; /**< Listening socket file descriptor */
    /* Shared state */
    /**
     * Callback for logging
     * Log callback format is (void* extra, const char* msg) where
     * extra -> User-defined data
     * msg -> Log message
     * */
    mint_atomicPtr_t log_closure;
    /**
     * Callback for forward
     * Forward callback format is (void* extra, const sn_packet_t* msg, sn_state_t* sns, sn_entry_t* nexthop) where
     * extra -> User-defined data
     * msg -> Message to be forwarded
     * sns -> Node state
     * nexthop -> Default nexthop, can be overwritten
     * */
    mint_atomicPtr_t forward_closure;
    /**
     * Callback for delivery
     * Forward callback format is (void* extra, const sn_packet_t* msg, sn_state_t* sns, sn_entry_t* nexthop) where
     * extra -> User-defined data
     * msg -> Message to be delivered
     * sns -> Node state
     * */
    mint_atomicPtr_t deliver_closure;
    /* Default closures */
    sn_closure_t default_log_closure; /**< Default log closure */
    sn_closure_t default_forward_closure; /**< Default forward closure */
    sn_closure_t default_deliver_closure; /**< Default deliver closure */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_SN_H_*/
