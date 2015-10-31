/**
 * @file
 * Implements a SecondNet node.
 * */

#ifndef SN_NODE_H_
#define SN_NODE_H_

#include "net/addr.h"
#include "net/router.h"
#include "io/sock.h"
#include "util/closure.h"
#include "crypto/sign.h"

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
typedef struct sn_node_t_ sn_node_t;

/**
 * Upcall callback type. Called when a message is received.
 * */
typedef int (*sn_upcall_t)(const unsigned char msg[], unsigned long long msg_len);

/**
 * Initializes a node
 * @param sns State to be initialized(must be already allocated)
 * @param sk Node secret key
 * @param pk Node public key and SecondNet address
 * @param socket Listening socket
 * @param check_sign If set messages not signed will be rejected
 * @return 0 if OK, -1 otherwise
 * */
int sn_node_at_socket(sn_node_t* sns, const sn_crypto_sign_key_t* sk, const sn_crypto_sign_pubkey_t* pk, const sn_io_sock_t socket, int check_sign);

/**
 * Initializes a node from a string address and a listening port
 * @param sns State to be initialized(must be already allocated)
 * @param sk Node secret key
 * @param pk Node public key and SecondNet address
 * @param port Listening port number.
 * @param check_sign If set messages not signed will be rejected
 * @return 0 if OK, -1 otherwise
 * */
int sn_node_at_port(sn_node_t* sns, const sn_crypto_sign_key_t* sk, const sn_crypto_sign_pubkey_t* pk, uint16_t port, int check_sign);

/**
 * Destroys a node
 * @param sns State to be destroyed(but not deallocated)
 * */
void sn_node_destroy(sn_node_t* sns);

/**
 * Sets the upcall
 * @param sns The state
 * @param upcall The new upcall. NULL to deregister the current upcall.
 * */
void sn_node_set_upcall(sn_node_t* sns, sn_upcall_t upcall);

/**
 * Call the upcall, if present.
 * @param sns Node state
 * @param msg Message
 * @param msg_len Message length
 * @return Upcall return, -1 if upcall is NULL
 * */
int sn_node_upcall(const sn_node_t* sns, const unsigned char msg[], unsigned long long msg_len);

/**
 * Changes the logging callback
 * @param sns Node state
 * @param cb The new callback. If it is NULL, default logging(stderr) will be used
 * */
void sn_node_set_log_callback(sn_node_t* sns, sn_util_closure_t* cb);

void sn_node_log(sn_node_t* sns, const char* format, ...);

/**
 * Changes the forwarding callback
 * @param sns Node state
 * @param cb The new callback. If it is NULL default forwarding will be used.
 * */
void sn_node_set_forward_callback(sn_node_t* sns, sn_util_closure_t* cb);

/**
 * Changes the delivering callback
 * @param sns Node state
 * @param cb The new callback. If it is NULL, message will be just logged.
 * */
void sn_node_set_deliver_callback(sn_node_t* sns, sn_util_closure_t* cb);

/**
 * Sends a message without need for acknowledgement
 * @param sns Node state
 * @param dst Destination address
 * @param len Message length
 * @param payload Message payload
 * @return -1 if error
 */
int sn_node_send(sn_node_t* sns, const sn_net_addr_t* dst, size_t len, const char* payload);

/**
 * Joins a SecondNet network using a know gateway
 * @param sns Node state
 * @param gateway Network address of the gateway
 * @return 0 if corretly joined, -1 otherwise
 * */
int sn_node_join(sn_node_t* sns, const sn_io_naddr_t* gateway);

struct sn_node_t_ {
    /* Background thread state */
    sn_net_addr_t self; /**< Node SecondNet address */
    sn_crypto_sign_key_t sk; /**< Node secret key*/
    sn_net_router_t router; /**< Routing state */
    pthread_t bg_thrd; /**< Background thread for routing */
    sn_io_sock_t socket; /**< Listening socket file descriptor */
    int sign; /**< Are signatures active? */
    int check_sign; /**< Are signature checks active? */
    /* Shared state */
    mint_atomicPtr_t upcall; /**< General upcall, received messages go up using this*/
    /**
     * Callback for logging
     * Log callback format is (void* extra, const char* msg) where
     * extra -> User-defined data
     * msg -> Log message
     * */
    mint_atomicPtr_t log_closure;
    /**
     * Callback for forward
     * Forward callback format is (void* extra, const sn_net_packet_t* msg, sn_node_t* sns, sn_net_entry_t* nexthop) where
     * extra -> User-defined data
     * msg -> Message to be forwarded
     * sns -> Node state
     * nexthop -> Default nexthop, can be overwritten
     * */
    mint_atomicPtr_t forward_closure;
    /**
     * Callback for delivery
     * Forward callback format is (void* extra, const sn_net_packet_t* msg, sn_node_t* sns, sn_net_entry_t* nexthop) where
     * extra -> User-defined data
     * msg -> Message to be delivered
     * sns -> Node state
     * */
    mint_atomicPtr_t deliver_closure;
    /* Default closures */
    sn_util_closure_t default_log_closure; /**< Default log closure */
    sn_util_closure_t default_forward_closure; /**< Default forward closure */
    sn_util_closure_t default_deliver_closure; /**< Default deliver closure */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_NODE_H_*/
