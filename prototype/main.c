#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/addr.h"
#include "net/packet.h"
#include "io/naddr.h"
#include "net/router.h"
#include "sndnet.h"

sn_state_t sns;

int main(int argc, char* argv[]) {
    char* self_addr;
    uint16_t port;
    char line[1024];
    char* command;

    if(argc < 3) {
        fprintf(stderr, "Usage: %s <self_addr> <port>\n", argv[0]);
        return 1;
    }

    self_addr = argv[1];
    sscanf(argv[2], "%hu", &port);

    if(sn_init_at_port(&sns, self_addr, port) == -1) {
        fprintf(stderr, "Error initializing\n");
        return 1;
    }

    while(printf("> "), fgets(line, 1024, stdin)) {
        command = strtok(line, " \n");

        if(!command)
            continue;

        if(strcmp(command, "quit") == 0) {
            break;
        }

        if(strcmp(command, "send") == 0) {
            char *dst, *payload;
            sn_net_addr_t sn_dst;

            dst = strtok(0, " \n");

            if(!dst) {
                printf("send <dst> <src> <payload>\n");
                continue;
            }

            sn_net_addr_from_hex(&sn_dst, dst);

            payload = strtok(0, "\n");

            if(!payload) {
                printf("send <dst> <payload>\n");
                continue;
            }

            sn_send(&sns, &sn_dst, strlen(payload), payload);
        }

        if(strcmp(command, "insert") == 0) {
            char *addr, *raddr;
            sn_net_addr_t sn_net_addr;
            sn_io_naddr_t sn_raddr;

            addr = strtok(0, " \n");

            if(!addr) {
                printf("insert <sn_net_addr> <addr(type:ip:port)>\n");
                continue;
            }

            raddr = strtok(0, " \n");

            if(!raddr) {
                printf("insert <sn_net_addr> <addr(type:ip:port)>\n");
                continue;
            }

            sn_net_addr_from_hex(&sn_net_addr, addr);
            sn_io_naddr_from_str(&sn_raddr, raddr);

            sn_net_router_add(&(sns.router), &sn_net_addr, &sn_raddr);
        }

        if(strcmp(command, "show") == 0) {
            char* buffer = malloc(30000);

            if(!buffer)
                return 1;

            sn_net_router_to_str(&sns.router, buffer, 30000);

            printf("%s\n", buffer);

            free(buffer);
        }
    }

    sn_destroy(&sns);

    return 0;
}
