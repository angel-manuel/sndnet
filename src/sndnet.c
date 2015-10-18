#include "sndnet.h"

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

#include "net/addr.h"
#include "common.h"
#include "net/packet.h"
#include "core.h"

int sn_deliver(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr);
int sn_forward(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr);
void sn_log(sn_state_t* sns, const char* format, ...);
void* sn_background(void* arg);

void call_log_cb(sn_state_t* sns, char* packet);
void call_forward_cb(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop);
void call_deliver_cb(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr);

void default_log_cb(int argc, void* argv[]);
void default_forward_cb(int argc, void* argv[]);
void default_deliver_cb(int argc, void* argv[]);

int sn_init(sn_state_t* sns, const sn_net_addr_t* self, const sn_io_sock_t socket) {
    sn_io_naddr_t self_net;

    assert(sns != NULL);
    assert(self != NULL);
    assert(socket != SN_IO_SOCK_INVALID);

    /* Sodium */

    if(sodium_init() == -1)
        return -1;

    /* Copying */

    sns->self = *self;
    sns->socket = socket;

    /* Callback registering */

    sn_set_upcall(sns, NULL);

    void* c_argv[] = { NULL };
    sn_util_closure_init_curried(&sns->default_log_closure, default_log_cb, 1, c_argv);
    sn_util_closure_init_curried(&sns->default_forward_closure, default_forward_cb, 1, c_argv);
    sn_util_closure_init_curried(&sns->default_deliver_closure, default_deliver_cb, 1, c_argv);
    sn_set_log_callback(sns, NULL);
    sn_set_forward_callback(sns, NULL);
    sn_set_deliver_callback(sns, NULL);

    /* Initializing */

    if(sn_io_sock_get_name(socket, &self_net) == -1)
        return -1;

    sn_net_router_init(&sns->router, &sns->self, &self_net);

    /* Background thread initialization */

    if(pthread_create(&(sns->bg_thrd), 0, sn_background, sns)) {
        sn_log(sns, "Error while starting thread");
        return -1;
    }

    return 0;
}

int sn_init_at_port(sn_state_t* sns, const char hexaddr[SN_NET_ADDR_PRINTABLE_LEN], uint16_t port) {
    sn_net_addr_t self;
    sn_io_naddr_t net_self;
    sn_io_sock_t socket;

    assert(sns != NULL);
    assert(hexaddr != NULL);

    sn_net_addr_from_hex(&self, hexaddr);

    if(sn_io_naddr_ipv4(&net_self, "0.0.0.0", port) == -1)
        goto error;

    if((socket = sn_io_sock_named(&net_self)) == SN_IO_SOCK_INVALID)
        goto error;

    if(sn_init(sns, &self, socket) == -1)
        goto error_lib_socket;

    return 0;

error_lib_socket:
    sn_io_sock_close(socket);
error:
    return -1;
}

void sn_destroy(sn_state_t* sns) {
    assert(sns != NULL);

    /* Thread closing */

    pthread_cancel(sns->bg_thrd);
    pthread_join(sns->bg_thrd, 0);

    /* Socket closing */

    sn_io_sock_close(sns->socket);
}

void sn_set_upcall(sn_state_t* sns, sn_upcall_t upcall) {
    assert(sns != NULL);

    mint_store_ptr_relaxed(&sns->upcall, upcall);
    mint_thread_fence_release();
}

void sn_upcall(const sn_state_t* sns, const unsigned char msg[], unsigned long long msg_len) {
    sn_upcall_t up;

    assert(sns != NULL);
    assert(msg != NULL);

    mint_thread_fence_acquire();
    up = (sn_upcall_t)mint_load_ptr_relaxed(&sns->upcall);

    if(up)
        up(msg, msg_len);
}

void sn_set_log_callback(sn_state_t* sns, sn_util_closure_t* closure) {
    sn_util_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_log_closure;
    mint_store_ptr_relaxed(&sns->log_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_log_cb(sn_state_t* sns, char* packet) {
    void* argv[] = { packet };

    assert(packet != NULL);

    mint_thread_fence_acquire();
    sn_util_closure_call(mint_load_ptr_relaxed(&sns->log_closure), 1, argv);
}

void sn_set_forward_callback(sn_state_t* sns, sn_util_closure_t* closure) {
    sn_util_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_forward_closure;
    mint_store_ptr_relaxed(&sns->forward_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_forward_cb(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop) {
    void* argv[] = { packet, sns, rem_addr, nexthop };

    assert(packet != NULL);
    assert(sns != NULL);
    assert(nexthop != NULL);

    mint_thread_fence_acquire();
    sn_util_closure_call(mint_load_ptr_relaxed(&sns->forward_closure), 4, argv);
}

void sn_set_deliver_callback(sn_state_t* sns, sn_util_closure_t* closure) {
    sn_util_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_log_closure;
    mint_store_ptr_relaxed(&sns->log_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_deliver_cb(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr) {
    void* argv[] = { packet, sns, rem_addr };

    assert(packet != NULL);
    assert(sns != NULL);

    mint_thread_fence_acquire();
    sn_util_closure_call(mint_load_ptr_relaxed(&sns->deliver_closure), 3, argv);
}

int sn_send(sn_state_t* sns, const sn_net_addr_t* dst, size_t len, const char* payload) {
    sn_net_packet_t* packet;

    assert(sns != NULL);
    assert(dst != NULL);
    assert(payload != NULL || len == 0);

    packet = sn_net_packet_pack(dst, &(sns->self), len, payload);

    if(!packet)
        return -1;

    if(sn_forward(sns, packet, NULL) == -1)
        return -1;

    free(packet);

    return 0;
}

int sn_join(sn_state_t* sns, const sn_io_naddr_t* gateway) {
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

int sn_deliver(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr) {
    assert(sns != NULL);
    assert(packet != NULL);

    call_deliver_cb(sns, packet, rem_addr);

    return sn_core_deliver(sns, packet, rem_addr);
}

int sn_forward(sn_state_t* sns, sn_net_packet_t* packet, sn_io_naddr_t* rem_addr) {
    sn_net_addr_t dst;
    sn_net_entry_t nexthop;

    assert(sns != NULL);
    assert(packet != NULL);

    sn_net_packet_get_dst(packet, &dst);

    sn_net_router_nexthop(&(sns->router), &dst, &nexthop);

    if(nexthop.is_set) {
        call_forward_cb(sns, packet, rem_addr, &nexthop);

        return sn_core_forward(sns, packet, rem_addr, &nexthop);

        return 0;
    } else {
        return sn_deliver(sns, packet, rem_addr);
    }
}

void sn_log(sn_state_t* sns, const char* format, ...) {
    char str[1024];
    va_list args;

    assert(sns != NULL);
    assert(format != NULL);

    va_start(args, format);
    vsnprintf(str, 1024, format, args);
    va_end(args);

    call_log_cb(sns, str);
}

void* sn_background(void* arg) {
    sn_state_t* sns = (sn_state_t*)arg;
    sn_io_naddr_t rem_addr;
    sn_net_packet_t* packet;

    assert(sns != NULL);

    do {
        packet = sn_net_packet_recv(sns->socket, &rem_addr);

        if(!packet)
            continue;

        sn_forward(sns, packet, &rem_addr);

        free(packet);
    } while(1);

    return sns;
}

/* Predifined callback */

void sn_silent_log_callback(int argc, void* argv[]) {
    SN_UNUSED(argc);
    SN_UNUSED(argv);
}

void sn_named_log_callback(int argc, void* argv[]) {
    assert(argc >= 2);
    assert(argv[0] != NULL);
    assert(argv[1] != NULL);

    fprintf(stderr, "%s: %s\n", (char*)argv[0], (char*)argv[1]);
}

/* Default callbacks */

void default_log_cb(int argc, void* argv[]) {
    assert(argc >= 2);
    assert(argv[1] != NULL);

    fprintf(stderr, "sndnet: %s\n", (char*)argv[1]);
}

void default_forward_cb(int argc, void* argv[]) {
    char nh_addr[SN_NET_ADDR_PRINTABLE_LEN];
    char nh_raddr[SN_IO_NADDR_PRINTABLE_LEN];
    char packet_str[SN_NET_PACKET_PRINTABLE_LEN];
    const sn_net_packet_t* packet = argv[1];
    sn_state_t* sns = argv[2];
    sn_io_naddr_t* rem_addr = argv[3];
    sn_net_entry_t* nexthop = argv[4];

    assert(argc >= 5);
    assert(packet != NULL);
    assert(sns != NULL);
    assert(nexthop != NULL);

    sn_net_addr_to_str(&nexthop->sn_net_addr, nh_addr);
    sn_io_naddr_to_str(&nexthop->net_addr, nh_raddr);
    sn_net_packet_header_to_str(packet, packet_str);

    if(rem_addr == NULL)
        sn_log(sns,
            "packet forward\n"
            "sent from THIS\n"
            "%s"
            "Forwarding to %s @ %s\n",
            packet_str, nh_addr, nh_raddr);
    else {
        char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];

        sn_io_naddr_to_str(rem_addr, rem_addr_str);

        sn_log(sns,
            "packet forward\n"
            "sent from %s\n"
            "%s"
            "Forwarding to %s @ %s\n",
            rem_addr_str, packet_str, nh_addr, nh_raddr);
    }
}

void default_deliver_cb(int argc, void* argv[]) {
    char packet_str[SN_NET_PACKET_PRINTABLE_LEN];
    const sn_net_packet_t* packet = argv[1];
    sn_state_t* sns = argv[2];
    sn_io_naddr_t* rem_addr = argv[3];

    assert(argc >= 4);
    assert(packet != NULL);
    assert(sns != NULL);

    sn_net_packet_header_to_str(packet, packet_str);

    if(rem_addr == NULL)
        sn_log(sns,
            "packet deliver\n"
            "sent from THIS\n"
            "%s"
            "\n%s\n",
        packet_str, packet->payload);
    else {
        char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];

        sn_io_naddr_to_str(rem_addr, rem_addr_str);

        sn_log(sns,
            "packet deliver\n"
            "sent from %s\n"
            "%s"
            "\n%s\n",
        rem_addr_str, packet_str, packet->payload);
    }
}
