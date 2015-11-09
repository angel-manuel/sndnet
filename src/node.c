#include "node.h"

#include <assert.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sodium.h>
#define asm __asm
#include <mintomic/mintomic.h>

#include "callbacks.h"
#include "net/addr.h"
#include "common.h"
#include "net/packet.h"
#include "handler.h"

int deliver(sn_node_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr);
int forward(sn_node_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr);
void* background(void* arg);

int upcall_wrapper(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);

void call_log_cb(sn_node_t* sns, char* packet);
void call_forward_cb(sn_node_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop);
void call_deliver_cb(sn_node_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr);

int sn_node_at_socket(sn_node_t* sns, const sn_crypto_sign_key_t* sk, const sn_crypto_sign_pubkey_t* pk, const sn_io_sock_t socket, int check_sign) {
    sn_io_naddr_t self_net;

    assert(sns != NULL);
    assert(sk != NULL || pk != NULL);
    assert(socket != SN_IO_SOCK_INVALID);

    /* Copying */

    sns->check_sign = check_sign;

    if(sk != NULL) {
        sns->sign = 1;
        sns->sk = *sk;
    } else {
        sns->sign = 0;
    }

    if(pk != NULL)
        sns->self = *((sn_net_addr_t*)pk);
    else if(sk != NULL)
        sn_crypto_sign_pk_from_sk(sk, (sn_crypto_sign_pubkey_t*)&sns->self);
    else
        return -1;

    sns->socket = socket;

    /* Callback registering */

    sn_node_set_upcall(sns, NULL);

    void* log_argv[] = { "sndnet" };
    sn_util_closure_init_curried(&sns->default_log_closure, sn_named_log_callback, 1, log_argv);
    sn_node_set_log_callback(sns, NULL);

    /* Initializing */

    if(sn_io_sock_get_name(socket, &self_net) == -1)
        return -1;

    sn_net_router_init(&sns->router, &sns->self, &self_net);

    /* Background thread initialization */

    if(pthread_create(&(sns->bg_thrd), 0, background, sns)) {
        sn_node_log(sns, "Error while starting thread");
        return -1;
    }

    return 0;
}

int sn_node_at_port(sn_node_t* sns, const sn_crypto_sign_key_t* sk, const sn_crypto_sign_pubkey_t* pk, uint16_t port, int check_sign) {
    sn_io_naddr_t net_self;
    sn_io_sock_t socket;

    assert(sns != NULL);
    assert(sk != NULL);

    if(sn_io_naddr_ipv4(&net_self, "0.0.0.0", port) == -1)
        goto error;

    if((socket = sn_io_sock_named(&net_self)) == SN_IO_SOCK_INVALID)
        goto error;

    if(sn_node_at_socket(sns, sk, pk, socket, check_sign) == -1)
        goto error_lib_socket;

    return 0;

error_lib_socket:
    sn_io_sock_close(socket);
error:
    return -1;
}

void sn_node_destroy(sn_node_t* sns) {
    assert(sns != NULL);

    /* Thread closing */

    pthread_cancel(sns->bg_thrd);
    pthread_join(sns->bg_thrd, 0);

    /* Socket closing */

    sn_io_sock_close(sns->socket);
}

void sn_node_set_upcall(sn_node_t* sns, sn_upcall_t upcall) {
    assert(sns != NULL);

    mint_store_ptr_relaxed(&sns->upcall, upcall);
    mint_thread_fence_release();
}

int sn_node_upcall(const sn_node_t* sns, const unsigned char msg[], unsigned long long msg_len) {
    sn_upcall_t up;

    assert(sns != NULL);
    assert(msg != NULL);

    mint_thread_fence_acquire();
    up = (sn_upcall_t)mint_load_ptr_relaxed(&sns->upcall);

    if(up)
        return up(msg, msg_len);

    return -1;
}

void sn_node_set_log_callback(sn_node_t* sns, sn_util_closure_t* closure) {
    sn_util_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_log_closure;
    mint_store_ptr_relaxed(&sns->log_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_log_cb(sn_node_t* sns, char* packet) {
    void* argv[] = { packet };

    assert(packet != NULL);

    mint_thread_fence_acquire();
    sn_util_closure_call(mint_load_ptr_relaxed(&sns->log_closure), 1, argv);
}

void sn_node_log(sn_node_t* sns, const char* format, ...) {
    char str[1024];
    va_list args;

    assert(sns != NULL);
    assert(format != NULL);

    va_start(args, format);
    vsnprintf(str, 1024, format, args);
    va_end(args);

    call_log_cb(sns, str);
}

int sn_node_send(sn_node_t* sns, const sn_net_addr_t* dst, size_t len, const char* payload) {
    return sn_node_send_typed(sns, dst, 0, len, payload);
}

int sn_node_send_typed(sn_node_t* sns, const sn_net_addr_t* dst, uint8_t type, size_t len, const char* payload) {
    sn_net_packet_t* packet;

    assert(sns != NULL);
    assert(dst != NULL);
    assert(payload != NULL || len == 0);
    assert(type < SN_WIRE_NET_TYPES);

    packet = sn_net_packet_pack(dst, &sns->self, type, len, payload);

    if(!packet)
        return -1;

    if(sns->sign)
        sn_net_packet_sign(packet, &sns->sk);

    if(forward(sns, packet, NULL) == -1)
        return -1;

    free(packet);

    return 0;
}

int sn_node_join(sn_node_t* sns, const sn_io_naddr_t* gateway) {
    assert(sns != NULL);
    assert(gateway != NULL);

    /*
    Join steps:
    1. Find m = owner(self)
    2. Get m leafset
    3. Fill our leafset
    4. Inform m leafset of ourselves
    5. Get traceroute to m
    6. Construct routing table for m traceroute
    */

    return 0;
}

/*Private functions*/

int deliver(sn_node_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr) {
    char packet_str[SN_NET_PACKET_PRINTABLE_LEN];
    char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];
    sn_deliver_handler_t d_fn;

    assert(sns != NULL);
    assert(packet != NULL);

    sn_net_packet_header_to_str(packet, packet_str);

    if(rem_addr != NULL)
        sn_io_naddr_to_str(rem_addr, rem_addr_str);
    else
        strncpy(rem_addr_str, "THIS", SN_IO_NADDR_PRINTABLE_LEN);

    sn_node_log(sns,
        "packet deliver\n"
        "came from %s\n"
        "%s"
        "\n%s\n",
    rem_addr_str, packet_str, packet->payload);

    //TODO: logic

    if(packet->header.type >= SN_WIRE_NET_TYPES)
        return -1;

    d_fn = sn_default_deliver_handlers[packet->header.type];

    if(d_fn)
        return d_fn(sns, packet, rem_addr);

    return 0;
}

int forward(sn_node_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr) {
    char nh_str[SN_NET_ENTRY_PRINTABLE_LEN];
    char src_str[SN_NET_ADDR_PRINTABLE_LEN];
    char dst_str[SN_NET_ADDR_PRINTABLE_LEN];
    char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];
    sn_net_addr_t src;
    sn_net_addr_t dst;
    sn_net_entry_t nexthop;

    assert(sns != NULL);
    assert(packet != NULL);

    sn_net_packet_get_src(packet, &src);
    sn_net_addr_to_str(&src, src_str);
    sn_net_packet_get_dst(packet, &dst);
    sn_net_addr_to_str(&dst, dst_str);

    if(rem_addr != NULL)
        sn_io_naddr_to_str(rem_addr, rem_addr_str);
    else
        strncpy(rem_addr_str, "THIS", SN_IO_NADDR_PRINTABLE_LEN);

    if(!packet->header.ttl) {
        sn_node_log(sns,
        "packet without TTL\n"
        "came from %s\n"
        "src: %s\n"
        "dst: %s\n",
        rem_addr_str, src_str, dst_str);
        return -1;
    }

    packet->header.ttl--;

    sn_net_router_nexthop(&(sns->router), &dst, &nexthop);
    sn_net_entry_to_str(&nexthop, nh_str, SN_NET_ADDR_HEX_LEN);

    if(nexthop.is_set) {
        sn_forward_handler_t f_fn;

        if(packet->header.type >= SN_WIRE_NET_TYPES)
            return -1;

        f_fn = sn_default_forward_handlers[packet->header.type];

        if(f_fn) {
            if(f_fn(sns, packet, rem_addr, &nexthop) != 0)
                return -1;

            if(!nexthop.is_set)
                return deliver(sns, packet, rem_addr);
        }

        if(sn_net_packet_send(packet, sns->socket, &nexthop.net_addr) == -1) {
            sn_node_log(sns, "ERROR sending packet to %s\n", rem_addr_str);
            return -1;
        } else {
            char packet_str[SN_NET_PACKET_PRINTABLE_LEN];

            sn_net_packet_header_to_str(packet, packet_str);
        
            sn_node_log(sns,
            "packet forward\n"
            "came from %s\n"
            "%s"
            "Forwarded to %s\n",
            rem_addr_str, packet_str, nh_str);
        }

        return 0;
    } else {
        return deliver(sns, packet, rem_addr);
    }
}

void* background(void* arg) {
    sn_node_t* sns = (sn_node_t*)arg;
    sn_io_naddr_t rem_addr;
    sn_net_packet_t* packet;

    assert(sns != NULL);

    do {
        packet = sn_net_packet_recv(sns->socket, &rem_addr);

        if(!packet)
            continue;

        if(sns->check_sign && sn_net_packet_check_sign(packet) != 0) {
            char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];
            char packet_str[SN_NET_PACKET_PRINTABLE_LEN];

            sn_io_naddr_to_str(&rem_addr, rem_addr_str);

            sn_net_packet_header_to_str(packet, packet_str);

            sn_node_log(sns,
                "Bad signed msg:\n"
                "sent from %s\n"
                "%s"
                "REJECTED\n",
                rem_addr_str, packet_str);
            continue;
        }

        forward(sns, packet, &rem_addr);

        free(packet);
    } while(1);

    return sns;
}

/*Deliver handlers*/

int upcall_wrapper(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr) {
    assert(sns != NULL);
    assert(packet != NULL);
    assert(rem_addr != NULL);

    return sn_node_upcall(sns, packet->payload, packet->header.len);
}

