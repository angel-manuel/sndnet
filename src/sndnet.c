#include "sndnet.h"

#include <assert.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "addr.h"
#include "msg.h"
#include "msg_type.h"

int sn_send_typed(sn_state_t* sns, const sn_addr_t* dst, size_t len, const char* payload, sn_msg_type_t type);
void sn_deliver(sn_state_t* sns, const sn_msg_t* msg);
int sn_forward(sn_state_t* sns, sn_msg_t* msg);
void sn_log(sn_state_t* sns, const char* format, ...);
void* sn_background(void* arg);

int on_msg_query_table(sn_state_t* sns, const sn_msg_t* msg);
int on_msg_query_leafset(sn_state_t* sns, const sn_msg_t* msg);
int on_msg_query_result(sn_state_t* sns, const sn_msg_t* msg);

void default_log_cb(const char* msg);
void default_forward_cb(const sn_msg_t* msg, sn_state_t* sns, sn_entry_t* nexthop);
void default_deliver_cb(const sn_msg_t* msg, sn_state_t* sns);

int sn_init(sn_state_t* sns, const sn_addr_t* self, const sn_sock_t* socket) {
    assert(sns != 0);

    /* Copying */

    sns->self = *self;
    sns->log_cb = default_log_cb;
    sns->deliver_cb = default_deliver_cb;
    sns->forward_cb = default_forward_cb;
    sns->socket = *socket;

    /* Initializing */

    sn_log(sns, "Initializing");

    sn_router_init(&sns->router, &sns->self, 0);

    /* Background thread initialization */

    if(pthread_create(&(sns->bg_thrd), 0, sn_background, sns)) {
        sn_log(sns, "Error while starting thread");
        return 1;
    }

    sn_log(sns, "Initialized");

    return 0;
}

void sn_destroy(sn_state_t* sns) {
    assert(sns != 0);

    sn_log(sns, "Destroying");

    /* Socket closing */

    sn_sock_destroy(&sns->socket);

    /* Thread closing */

    pthread_cancel(sns->bg_thrd);
    pthread_join(sns->bg_thrd, 0);

    sn_log(sns, "Destroyed");
}

void sn_set_log_callback(sn_state_t* sns, sn_log_callback cb) {
    assert(sns != 0);

    if(cb)
        sns->log_cb = cb;
    else
        sns->log_cb = default_log_cb;

    sn_log(sns, "Log callback changed");
}

void sn_set_forward_callback(sn_state_t* sns, sn_forward_callback cb) {
    assert(sns != 0);

    if(cb)
        sns->forward_cb = cb;
    else
        sns->forward_cb = default_forward_cb;

    sn_log(sns, "Forwarding callback changed");
}

void sn_set_deliver_callback(sn_state_t* sns, sn_deliver_callback cb) {
    assert(sns != 0);

    if(cb)
        sns->deliver_cb = cb;
    else
        sns->deliver_cb = default_deliver_cb;

    sn_log(sns, "Delivering callback changed");
}

int sn_send(sn_state_t* sns, const sn_addr_t* dst, size_t len, const char* payload) {
    assert(sns != 0);
    assert(dst != 0);
    assert(payload != 0 || len == 0);

    return sn_send_typed(sns, dst, len, payload, SN_MSG_TYPE_USER);
}

int sn_join(sn_state_t* sns, const sn_netaddr_t* gateway) {
    assert(sns != 0);
    assert(gateway != 0);

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

    assert(sns != 0);
    assert(dst != 0);
    assert(payload != 0 || len == 0);

    msg = sn_msg_pack(dst, &(sns->self), type, len, payload);

    if(!msg)
        return -1;

    if(sn_forward(sns, msg) == -1)
        return -1;

    free(msg);

    return 0;
}

void sn_deliver(sn_state_t* sns, const sn_msg_t* msg) {
    assert(sns != 0);
    assert(msg != 0);

    switch (msg->header.type) {
        case SN_MSG_TYPE_USER:
            (sns->deliver_cb)(msg, sns);
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

    assert(sns != 0);
    assert(msg != 0);

    sn_msg_get_dst(msg, &dst);

    sn_router_nexthop(&(sns->router), &dst, &nexthop);

    if(nexthop.is_set) {
        if(!msg->header.ttl) {
            return -1;
        }

        msg->header.ttl--;

        (sns->forward_cb)(msg, sns, &nexthop);

        sent = sn_msg_send(msg, &sns->socket, &(nexthop.net_addr));

        if(sent <= 0) {
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

    assert(sns != 0);
    assert(sns->log_cb != 0);
    assert(format != 0);

    va_start(args, format);
    vsnprintf(str, 1024, format, args);
    va_end(args);

    sns->log_cb(str);
}

void* sn_background(void* arg) {
    sn_state_t* sns = (sn_state_t*)arg;
    sn_netaddr_t rem_addr;
    sn_msg_t* msg;

    assert(sns != 0);

    do {
        msg = sn_msg_recv(&sns->socket, &rem_addr);

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

    assert(sns != 0);
    assert(msg != 0);
    assert(msg->header.type == SN_MSG_TYPE_QUERY_TABLE);

    qsize = sn_router_query_table(&sns->router, query->min_level, query->max_level, &query_result);

    if(qsize == 0) {
        sn_log(sns, "Error on remote table query with query ID: %hu", query->query_id);
        return -1;
    }

    msg_result = (sn_msg_type_query_result_t*)malloc(sizeof(sn_msg_type_query_result_t) + qsize);

    if(msg_result == 0) {
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

    assert(sns != 0);
    assert(msg != 0);
    assert(msg->header.type == SN_MSG_TYPE_QUERY_LEAFSET);

    qsize = sn_router_query_leafset(&sns->router, query->min_position, query->max_position, &query_result);

    if(qsize == 0) {
        sn_log(sns, "Error on remote table query with query ID: %hu", query->query_id);
        return -1;
    }

    msg_result = (sn_msg_type_query_result_t*)malloc(sizeof(sn_msg_type_query_result_t) + qsize);

    if(msg_result == 0) {
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

    assert(sns != 0);
    assert(msg != 0);
    assert(msg->header.type == SN_MSG_TYPE_QUERY_RESULT);

    res = (sn_router_query_ser_t*)msg->payload;

    for(uint32_t i = 0; i < res->entries_len; ++i) {
        if(sn_entry_deser(&e, &res->entries[i].entry) == 0)
            sn_router_add(&sns->router, &e.sn_addr, &e.net_addr);
    }

    return 0;
}

/* Default callbacks */

void default_log_cb(const char* msg) {
    assert(msg != 0);

    fprintf(stderr, "sndnet: %s\n", msg);
}

void default_forward_cb(const sn_msg_t* msg, sn_state_t* sns, sn_entry_t* nexthop) {
    char nh_addr[SN_ADDR_PRINTABLE_LEN];
    char nh_raddr[SN_NETADDR_PRINTABLE_LEN];

    assert(msg != 0);
    assert(sns != 0);
    assert(nexthop != 0);

    sn_addr_tostr(&nexthop->sn_addr, nh_addr);
    sn_netaddr_tostr(&nexthop->net_addr, nh_raddr);

    sn_log(sns, "Forwarding to %s @ %s\n", nh_addr, nh_raddr);
}

void default_deliver_cb(const sn_msg_t* msg, sn_state_t* sns) {
    char msg_str[SN_MSG_PRINTABLE_LEN];

    assert(msg != 0);
    assert(sns != 0);

    sn_msg_header_tostr(msg, msg_str);

    sn_log(sns,
        "msg deliver\n"
        "%s"
        "\n%s\n",
    msg_str, msg->payload);
}
