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

#include "address.h"
#include "message.h"
#include "message_type.h"

void sn_deliver(sn_state_t* sns, const sn_message_t* msg);
int sn_forward(sn_state_t* sns, sn_message_t* msg);
void sn_log(sn_state_t* sns, const char* format, ...);
void* sn_background(void* arg);

void default_log_cb(const char* msg);
void default_forward_cb(const sn_message_t* msg, sn_state_t* sns, sn_entry_t* nexthop);
void default_deliver_cb(const sn_message_t* msg, sn_state_t* sns);

int sn_init(sn_state_t* sns, const sn_addr_t* self, unsigned short port) {
    int socket_fd;
    struct sockaddr_in serv_addr;
    
    assert(sns != 0);
    
    /* Copying */
    
    sns->self = *self;
    sn_router_init(&(sns->router), &(sns->self));
    sns->log_cb = default_log_cb;
    sns->deliver_cb = default_deliver_cb;
    sns->forward_cb = default_forward_cb;
    sns->port = port;
    
    sn_log(sns, "Initializing");
    
    /* Socket initialization */
    
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(socket_fd == -1) {
        sn_log(sns, "Error while initializing socket");
        return 1;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    
    if(bind(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        sn_log(sns, "Error while initializing socket");
        close(socket_fd);
        return 1;
    }
    
    sns->socket_fd = socket_fd;
    
    /* Background thread initialization */
    
    if(pthread_create(&(sns->bg_thrd), 0, sn_background, sns)) {
        sn_log(sns, "Error while starting thread");
        close(socket_fd);
        return 1;
    }
    
    sn_log(sns, "Initialized");
    
    return 0;
}

void sn_destroy(sn_state_t* sns) {
    assert(sns != 0);
    
    sn_log(sns, "Destroying");
    
    /* Socket closing */
    
    close(sns->socket_fd);
    
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
    sn_message_t* msg;

    assert(sns != 0);
    assert(dst != 0);
    assert(payload != 0 || len == 0);

    msg = sn_message_pack(dst, &(sns->self), SN_MSG_TYPE_USER, len, payload);

    if(!msg)
        return -1;

    sn_forward(sns, msg);

    free(msg);

    return 0;
}

int sn_join(sn_state_t* sns, const sn_realaddr_t* gateway) {
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

void sn_deliver(sn_state_t* sns, const sn_message_t* msg) {
    assert(sns != 0);
    assert(msg != 0);

    (sns->deliver_cb)(msg, sns);
}

int sn_forward(sn_state_t* sns, sn_message_t* msg) {
    sn_addr_t dst;
    sn_entry_t nexthop;
    int sent;

    assert(sns != 0);
    assert(msg != 0);

    sn_addr_init(&dst, msg->header.dst);

    sn_router_nexthop(&(sns->router), &dst, &nexthop);

    if(nexthop.is_set) {
        if(!msg->header.ttl) {
            return -1;
        }
        
        msg->header.ttl--;
        
        (sns->forward_cb)(msg, sns, &nexthop);

        sent = sn_message_send(msg, sns->socket_fd, &(nexthop.net_addr));

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
    sn_realaddr_t rem_addr;
    sn_message_t* msg;
    
    assert(sns != 0);
    
    do {
        msg = sn_message_recv(sns->socket_fd, &rem_addr);

        if(!msg)
            continue;

        sn_forward(sns, msg);
        
        free(msg);
    } while(1);
    
    return sns;
}

/*Default callbacks*/


void default_log_cb(const char* msg) {
    assert(msg != 0);

    fprintf(stderr, "sndnet: %s\n", msg);
}

void default_forward_cb(const sn_message_t* msg, sn_state_t* sns, sn_entry_t* nexthop) {
    char nh_addr[SN_ADDR_PRINTABLE_LEN];
    char nh_raddr[SN_REALADDR_PRINTABLE_LEN];

    assert(msg != 0);
    assert(sns != 0);
    assert(nexthop != 0);

    sn_addr_tostr(&(nexthop->sn_addr), nh_addr);
    sn_realaddr_tostr(&(nexthop->net_addr), nh_raddr);

    sn_log(sns, "Forwarding to %s @ %s\n", nh_addr, nh_raddr);
}

void default_deliver_cb(const sn_message_t* msg, sn_state_t* sns) {
    sn_addr_t dst, src;
    char dst_str[SN_ADDR_PRINTABLE_LEN];
    char src_str[SN_ADDR_PRINTABLE_LEN];

    sn_addr_init(&dst, msg->header.dst);
    sn_addr_init(&src, msg->header.src);

    sn_addr_tostr(&dst, dst_str);
    sn_addr_tostr(&src, src_str);

    sn_log(sns,
        "msg\n"
        "dst = %s\n"
        "src = %s\n"
        "ttl = %hu\n"
        "len = %u\n"
        "buf = %s\n",
    dst_str,
    src_str,
    msg->header.ttl, msg->header.len, msg->payload);
}
