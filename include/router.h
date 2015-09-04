#ifndef SNDNET_ROUTER_H_
#define SNDNET_ROUTER_H_

#include "address.h"

#include <sys/socket.h>
#include <sys/types.h>

#define SNDNET_ROUTER_LEVELS (SNDNET_ADDRESS_LENGTH*2)
#define SNDNET_ROUTER_COLUMNS 16
#define SNDNET_ROUTER_LEAFSET 8
#define SNDNET_ROUTER_NEIGHBOURHOOD 8

typedef struct SNEntry_ {
	unsigned char is_set;
	SNAddress sn_addr;
	struct sockaddr net_addr;
} SNEntry;

typedef struct SNRouter_ {
	SNAddress self;
	SNEntry table[SNDNET_ROUTER_LEVELS][SNDNET_ROUTER_COLUMNS];
	SNEntry leafset[SNDNET_ROUTER_LEAFSET];
	SNEntry neighbourdhood[SNDNET_ROUTER_NEIGHBOURHOOD];
} SNRouter;

void sndnet_router_init(SNRouter* snr, const SNAddress* self);
void sndnet_router_destroy(SNRouter* snr);
void sndnet_router_add(SNRouter* snr, const SNEntry* sne);
void sndnet_router_nexthop(SNRouter* snr, const SNAddress* dst, SNEntry* nexthop);

#endif/*SNDNET_ROUTER_H_*/
