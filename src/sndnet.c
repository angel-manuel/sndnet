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

#include "addr.h"
#include "msg.h"
#include "msg_type.h"

int sn_send_typed(sn_state_t* sns, const sn_addr_t* dst, size_t len, const char* payload, sn_msg_type_t type);
void sn_deliver(sn_state_t* sns, sn_msg_t* msg);
int sn_forward(sn_state_t* sns, sn_msg_t* msg);
void sn_log(sn_state_t* sns, const char* format, ...);
void* sn_background(void* arg);

int on_msg_query_table(sn_state_t* sns, const sn_msg_t* msg);
int on_msg_query_leafset(sn_state_t* sns, const sn_msg_t* msg);
int on_msg_query_result(sn_state_t* sns, const sn_msg_t* msg);

void call_log_cb(sn_state_t* sns, char* msg);
void call_forward_cb(sn_state_t* sns, sn_msg_t* msg, sn_entry_t* nexthop);
void call_deliver_cb(sn_state_t* sns, sn_msg_t* msg);

void default_log_cb(int argc, void* argv[]);
void default_forward_cb(int argc, void* argv[]);
void default_deliver_cb(int argc, void* argv[]);

int sn_init(sn_state_t* sns, const sn_addr_t* self, const sn_io_sock_t socket) {
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

    sn_router_init(&sns->router, &sns->self, &self_net);

    /* Background thread initialization */

    if(pthread_create(&(sns->bg_thrd), 0, sn_background, sns)) {
        sn_log(sns, "Error while starting thread");
        return -1;
    }

    return 0;
}

int sn_init_at_port(sn_state_t* sns, const char hexaddr[SN_ADDR_PRINTABLE_LEN], uint16_t port) {
    sn_addr_t self;
    sn_io_naddr_t net_self;
    sn_io_sock_t socket;

    assert(sns != NULL);
    assert(hexaddr != NULL);

    sn_addr_from_hex(&self, hexaddr);

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

inline void call_forward_cb(sn_state_t* sns, sn_msg_t* msg, sn_entry_t* nexthop) {
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

inline void call_deliver_cb(sn_state_t* sns, sn_msg_t* msg) {
    void* argv[] = { msg, sns };

    assert(msg != NULL);
    assert(sns != NULL);

    mint_thread_fence_acquire();
    sn_closure_call(mint_load_ptr_relaxed(&sns->deliver_closure), 2, argv);
}

int sn_send(sn_state_t* sns, const sn_addr_t* dst, size_t len, const char* payload) {
    assert(sns != NULL);
    assert(dst != NULL);
    assert(payload != NULL || len == 0);

    return sn_send_typed(sns, dst, len, payload, SN_MSG_TYPE_USER);
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

int sn_send_typed(sn_state_t* sns, const sn_addr_t* dst, size_t len, const char* payload, sn_msg_type_t type) {
    sn_msg_t* msg;

    assert(sns != NULL);
    assert(dst != NULL);
    assert(payload != NULL || len == 0);

    msg = sn_msg_pack(dst, &(sns->self), type, len, payload);

    if(!msg)
        return -1;

    if(sn_forward(sns, msg) == -1)
        return -1;

    free(msg);

    return 0;
}

void sn_deliver(sn_state_t* sns, sn_msg_t* msg) {
    assert(sns != NULL);
    assert(msg != NULL);

    switch (msg->header.type) {
        case SN_MSG_TYPE_USER:
            call_deliver_cb(sns, msg);
            break;
        case SN_MSG_TYPE_QUERY_TABLE:
            on_msg_query_table(sns, msg);
            break;
        case SN_MSG_TYPE_QUERY_LEAFSET:
            on_msg_query_leafset(sns, msg);
            break;
        case SN_MSG_TYPE_QUERY_RESULT:
            on_msg_query_result(sns, msg);
            break;
        default:
            sn_log(sns, "Error: unknown message type: %hu", msg->header.type);
    }
}

int sn_forward(sn_state_t* sns, sn_msg_t* msg) {
    sn_addr_t dst;
    sn_entry_t nexthop;
    int sent;

    assert(sns != NULL);
    assert(msg != NULL);

    sn_msg_get_dst(msg, &dst);

    sn_router_nexthop(&(sns->router), &dst, &nexthop);

    if(nexthop.is_set) {
        if(!msg->header.ttl) {
            return -1;
        }

        msg->header.ttl--;

        call_forward_cb(sns, msg, &nexthop);

        sent = sn_msg_send(msg, sns->socket, &(nexthop.net_addr));

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
    sn_msg_t* msg;

    assert(sns != NULL);

    do {
        msg = sn_msg_recv(sns->socket, &rem_addr);

        if(!msg)
            continue;

        sn_forward(sns, msg);

        free(msg);
    } while(1);

    return sns;
}

/* Message handlers */

int on_msg_query_table(sn_state_t* sns, const sn_msg_t* msg) {
    const sn_msg_type_query_table_t* query = (sn_msg_type_query_table_t*)msg->payload;
    sn_router_query_ser_t* query_result;
    size_t qsize;
    sn_msg_type_query_result_t* msg_result;

    assert(sns != NULL);
    assert(msg != NULL);
    assert(msg->header.type == SN_MSG_TYPE_QUERY_TABLE);

    qsize = sn_router_query_table(&sns->router, query->min_level, query->max_level, &query_result);

    if(qsize == 0) {
        sn_log(sns, "Error on remote table query with query ID: %hu", query->query_id);
        return -1;
    }

    msg_result = (sn_msg_type_query_result_t*)malloc(sizeof(sn_msg_type_query_result_t) + qsize);

    if(msg_result == NULL) {
        sn_log(sns, "Error on malloc");
        free(query_result);
        return -1;
    }

    msg_result->query_id = query->query_id;
    memcpy(&msg_result->result, query_result, qsize);

    free(query_result);

    if(sn_send_typed(sns, (sn_addr_t*)&msg->header.src, sizeof(sn_msg_type_query_result_t) + qsize, (const char*)msg_result, SN_MSG_TYPE_QUERY_RESULT) == -1) {
        sn_log(sns, "Error while sending query result");
        free(msg_result);
        return -1;
    }

    free(msg_result);

    return 0;
}

int on_msg_query_leafset(sn_state_t* sns, const sn_msg_t* msg) {
    const sn_msg_type_query_leafset_t* query = (sn_msg_type_query_leafset_t*)msg->payload;
    sn_router_query_ser_t* query_result;
    size_t qsize;
    sn_msg_type_query_result_t* msg_result;

    assert(sns != NULL);
    assert(msg != NULL);
    assert(msg->header.type == SN_MSG_TYPE_QUERY_LEAFSET);

    qsize = sn_router_query_leafset(&sns->router, query->min_position, query->max_position, &query_result);

    if(qsize == 0) {
        sn_log(sns, "Error on remote table query with query ID: %hu", query->query_id);
        return -1;
    }

    msg_result = (sn_msg_type_query_result_t*)malloc(sizeof(sn_msg_type_query_result_t) + qsize);

    if(msg_result == NULL) {
        sn_log(sns, "Error on malloc");
        free(query_result);
        return -1;
    }

    msg_result->query_id = query->query_id;
    memcpy(&msg_result->result, query_result, qsize);

    free(query_result);

    if(sn_send_typed(sns, (sn_addr_t*)&msg->header.src, sizeof(sn_msg_type_query_result_t) + qsize, (const char*)msg_result, SN_MSG_TYPE_QUERY_RESULT) == -1) {
        sn_log(sns, "Error while sending query result");
        free(msg_result);
        return -1;
    }

    free(msg_result);

    return 0;
}

int on_msg_query_result(sn_state_t* sns, const sn_msg_t* msg) {
    sn_entry_t e;
    const sn_router_query_ser_t* res;

    assert(sns != NULL);
    assert(msg != NULL);
    assert(msg->header.type == SN_MSG_TYPE_QUERY_RESULT);

    res = (sn_router_query_ser_t*)msg->payload;

    for(uint32_t i = 0; i < res->entries_len; ++i) {
        if(sn_entry_deser(&e, &res->entries[i].entry) == 0)
            sn_router_add(&sns->router, &e.sn_addr, &e.net_addr);
    }

    return 0;
}

/* Predifined callback */

void sn_silent_log_callback(int argc, void* argv[]) {
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
    char nh_addr[SN_ADDR_PRINTABLE_LEN];
    char nh_raddr[SN_IO_NADDR_PRINTABLE_LEN];
    char msg_str[SN_MSG_PRINTABLE_LEN];
    const sn_msg_t* msg = argv[1];
    sn_state_t* sns = argv[2];
    sn_entry_t* nexthop = argv[3];

    assert(argc >= 4);
    assert(msg != NULL);
    assert(sns != NULL);
    assert(nexthop != NULL);

    sn_addr_to_str(&nexthop->sn_addr, nh_addr);
    sn_io_naddr_to_str(&nexthop->net_addr, nh_raddr);
    sn_msg_header_to_str(msg, msg_str);

    sn_log(sns,
        "msg forward\n"
        "%s"
        "Forwarding to %s @ %s\n",
        msg_str, nh_addr, nh_raddr);
}

void default_deliver_cb(int argc, void* argv[]) {
    char msg_str[SN_MSG_PRINTABLE_LEN];
    const sn_msg_t* msg = argv[1];
    sn_state_t* sns = argv[2];

    assert(argc >= 3);
    assert(msg != NULL);
    assert(sns != NULL);

    sn_msg_header_to_str(msg, msg_str);

    sn_log(sns,
        "msg deliver\n"
        "%s"
        "\n%s\n",
    msg_str, msg->payload);
}
