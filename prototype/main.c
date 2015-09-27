#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "message.h"
#include "realaddress.h"
#include "router.h"
#include "sndnet.h"

int main(int argc, char* argv[]) {
    char* self_addr;
    uint16_t port;
    sndnet_state_t sns;
    sndnet_addr_t self;
    char line[1024];
    char* command;

    if(argc < 3) {
        fprintf(stderr, "Usage: %s <self_addr> <port>\n", argv[0]);
        return 1;
    }

    self_addr = argv[1];
    sscanf(argv[2], "%hu", &port);

    sndnet_address_from_hex(&self, self_addr);
    
    sndnet_init(&sns, &self, port);
    
    while(printf("> "), fgets(line, 1024, stdin)) {
        command = strtok(line, " \n");

        if(!command)
            continue;
        
        if(strcmp(command, "quit") == 0) {
            break;
        }

        if(strcmp(command, "send") == 0) {
            char *dst, *payload;
            sndnet_addr_t sn_dst;
            
            dst = strtok(0, " \n");

            if(!dst) {
                printf("send <dst> <src> <payload>\n");
                continue;
            }

            sndnet_address_from_hex(&sn_dst, dst);

            payload = strtok(0, "\n");

            if(!payload) {
                printf("send <dst> <payload>\n");
                continue;
            }

            sndnet_send(&sns, &sn_dst, strlen(payload), payload);
        }

        if(strcmp(command, "insert") == 0) {
            char *addr, *raddr;
            sndnet_addr_t sn_addr;
            sndnet_realaddr_t sn_raddr;

            addr = strtok(0, " \n");

            if(!addr) {
                printf("insert <sn_addr> <addr(ip:port)>\n");
                continue;
            }

            raddr = strtok(0, " \n");

            if(!raddr) {
                printf("insert <sn_addr> <addr(ip:port)>\n");
                continue;
            }

            sndnet_address_from_hex(&sn_addr, addr);
            sndnet_realaddress_from_str(&sn_raddr, raddr);

            sndnet_router_add(&(sns.router), &sn_addr, &sn_raddr);
        }
        
        if(strcmp(command, "show") == 0) {
            char* buffer = malloc(30000);
            
            if(!buffer)
                return 1;
            
            sndnet_router_tostr(&sns.router, buffer, 30000);
            
            printf("%s\n", buffer);
            
            free(buffer);
        }
    }
    
    sndnet_destroy(&sns);
    
    return 0;
}
