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

#define asm __asm
#include <mintomic/mintomic.h>

#include "net/addr.h"
#include "common.h"
#include "net/packet.h"

void sn_deliver(sn_state_t* sns, sn_net_packet_t* msg);
int sn_forward(sn_state_t* sns, sn_net_packet_t* msg);
void sn_log(sn_state_t* sns, const char* format, ...);
void* sn_background(void* arg);

void call_log_cb(sn_state_t* sns, char* msg);
void call_forward_cb(sn_state_t* sns, sn_net_packet_t* msg, sn_net_entry_t* nexthop);
void call_deliver_cb(sn_state_t* sns, sn_net_packet_t* msg);

void default_log_cb(int argc, void* argv[]);
void default_forward_cb(int argc, void* argv[]);
void default_deliver_cb(int argc, void* argv[]);

int sn_init(sn_state_t* sns, const sn_net_addr_t* self, const sn_io_sock_t socket) {
    sn_io_naddr_t self_net;

    assert(sns != NULL);
    assert(self != NULL);
    assert(socket != SN_IO_SOCK_INVALID);

    /* Copying */

    sns->self = *self;
    sns->socket = socket;

    /* Callback registering */

    void* c_argv[] = { NULL };
    sn_closure_init_curried(&sns->default_log_closure, default_log_cb, 1, c_argv);
    sn_closure_init_curried(&sns->default_forward_closure, default_forward_cb, 1, c_argv);
    sn_closure_init_curried(&sns->default_deliver_closure, default_deliver_cb, 1, c_argv);
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

void sn_set_log_callback(sn_state_t* sns, sn_closure_t* closure) {
    sn_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_log_closure;
    mint_store_ptr_relaxed(&sns->log_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_log_cb(sn_state_t* sns, char* msg) {
    void* argv[] = { msg };

    assert(msg != NULL);

    mint_thread_fence_acquire();
    sn_closure_call(mint_load_ptr_relaxed(&sns->log_closure), 1, argv);
}

void sn_set_forward_callback(sn_state_t* sns, sn_closure_t* closure) {
    sn_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_forward_closure;
    mint_store_ptr_relaxed(&sns->forward_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_forward_cb(sn_state_t* sns, sn_net_packet_t* msg, sn_net_entry_t* nexthop) {
    void* argv[] = { msg, sns, nexthop };

    assert(msg != NULL);
    assert(sns != NULL);
    assert(nexthop != NULL);

    mint_thread_fence_acquire();
    sn_closure_call(mint_load_ptr_relaxed(&sns->forward_closure), 3, argv);
}

void sn_set_deliver_callback(sn_state_t* sns, sn_closure_t* closure) {
    sn_closure_t *new_closure;

    assert(sns != NULL);

    new_closure = closure ? closure : &sns->default_log_closure;
    mint_store_ptr_relaxed(&sns->log_closure, new_closure);
    mint_thread_fence_release();
}

inline void call_deliver_cb(sn_state_t* sns, sn_net_packet_t* msg) {
    void* argv[] = { msg, sns };

    assert(msg != NULL);
    assert(sns != NULL);

    mint_thread_fence_acquire();
    sn_closure_call(mint_load_ptr_relaxed(&sns->deliver_closure), 2, argv);
}

int sn_send(sn_state_t* sns, const sn_net_addr_t* dst, size_t len, const char* payload) {
    sn_net_packet_t* msg;

    assert(sns != NULL);
    assert(dst != NULL);
    assert(payload != NULL || len == 0);

    msg = sn_net_packet_pack(dst, &(sns->self), len, payload);

    if(!msg)
        return -1;

    if(sn_forward(sns, msg) == -1)
        return -1;

    free(msg);

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

void sn_deliver(sn_state_t* sns, sn_net_packet_t* msg) {
    assert(sns != NULL);
    assert(msg != NULL);

    call_deliver_cb(sns, msg);
}

int sn_forward(sn_state_t* sns, sn_net_packet_t* msg) {
    sn_net_addr_t dst;
    sn_net_entry_t nexthop;
    int sent;

    assert(sns != NULL);
    assert(msg != NULL);

    sn_net_packet_get_dst(msg, &dst);

    sn_net_router_nexthop(&(sns->router), &dst, &nexthop);

    if(nexthop.is_set) {
        if(!msg->header.ttl) {
            return -1;
        }

        msg->header.ttl--;

        call_forward_cb(sns, msg, &nexthop);

        sent = sn_net_packet_send(msg, sns->socket, &(nexthop.net_addr));

        if(sent == -1) {
            msg->header.ttl++;
            return -1;
        }

        return 0;
    } else {
        sn_deliver(sns, msg);
        return 0;
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
    sn_net_packet_t* msg;

    assert(sns != NULL);

    do {
        msg = sn_net_packet_recv(sns->socket, &rem_addr);

        if(!msg)
            continue;

        sn_forward(sns, msg);

        free(msg);
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
    char msg_str[SN_NET_PACKET_PRINTABLE_LEN];
    const sn_net_packet_t* msg = argv[1];
    sn_state_t* sns = argv[2];
    sn_net_entry_t* nexthop = argv[3];

    assert(argc >= 4);
    assert(msg != NULL);
    assert(sns != NULL);
    assert(nexthop != NULL);

    sn_net_addr_to_str(&nexthop->sn_net_addr, nh_addr);
    sn_io_naddr_to_str(&nexthop->net_addr, nh_raddr);
    sn_net_packet_header_to_str(msg, msg_str);

    sn_log(sns,
        "msg forward\n"
        "%s"
        "Forwarding to %s @ %s\n",
        msg_str, nh_addr, nh_raddr);
}

void default_deliver_cb(int argc, void* argv[]) {
    char msg_str[SN_NET_PACKET_PRINTABLE_LEN];
    const sn_net_packet_t* msg = argv[1];
    sn_state_t* sns = argv[2];

    assert(argc >= 3);
    assert(msg != NULL);
    assert(sns != NULL);

    sn_net_packet_header_to_str(msg, msg_str);

    sn_log(sns,
        "msg deliver\n"
        "%s"
        "\n%s\n",
    msg_str, msg->payload);
}
