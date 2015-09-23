#include "catch.hpp"

#include <router.h>

SCENARIO("routing is correct", "router") {
    GIVEN("An empty router on 4f5e22") {
        sndnet_router_t r;
        sndnet_addr_t self;
        
        sndnet_address_from_hex(&self, "4f5e22");
        sndnet_router_init(&r, &self);
        
        WHEN("Asking for next hop to 888888(shouldn't return one)") {
            sndnet_addr_t dst;
            sndnet_entry_t nexthop;

            sndnet_address_from_hex(&dst, "888888");

            sndnet_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 0); //No nexthop, deliver to self
            REQUIRE(sndnet_address_cmp(&self, &(nexthop.sn_addr)) == 0);
        }
    }

    GIVEN("A router on 4f5e22 that knows of 888888") {
        sndnet_router_t r;
        sndnet_addr_t self;
        sndnet_addr_t rem;
        
        sndnet_address_from_hex(&self, "4f5e22");
        sndnet_router_init(&r, &self);

        sndnet_address_from_hex(&rem, "888888");

        sndnet_router_add(&r, &rem, NULL);
        
        WHEN("Asking for next hop to 888888") {
            sndnet_addr_t dst;
            sndnet_entry_t nexthop;

            sndnet_address_from_hex(&dst, "888888");

            sndnet_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 1);
            REQUIRE(sndnet_address_cmp(&rem, &(nexthop.sn_addr)) == 0);
        }

        WHEN("Asking for next hop to 488888") {
            sndnet_addr_t dst;
            sndnet_entry_t nexthop;

            sndnet_address_from_hex(&dst, "488888");

            sndnet_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 0); //No nexthop, deliver to self
            REQUIRE(sndnet_address_cmp(&self, &(nexthop.sn_addr)) == 0);
        }

        WHEN("Asking for next hop to 788888") {
            sndnet_addr_t dst;
            sndnet_entry_t nexthop;

            sndnet_address_from_hex(&dst, "788888");

            sndnet_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 1);
            REQUIRE(sndnet_address_cmp(&rem, &(nexthop.sn_addr)) == 0);
        }

        WHEN("Removing 888888 and asking 788888") {
            sndnet_addr_t dst;
            sndnet_entry_t nexthop;

            sndnet_router_remove(&r, &rem);

            sndnet_address_from_hex(&dst, "788888");

            sndnet_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 0);
            REQUIRE(sndnet_address_cmp(&self, &(nexthop.sn_addr)) == 0);
        }
    }
}
