#include "catch.hpp"

#include <router.h>

SCENARIO("routing is correct", "router") {
    GIVEN("An empty router on 4f5e22") {
        sn_router_t r;
        sn_addr_t self;
        
        sn_addr_from_hex(&self, "4f5e22");
        sn_router_init(&r, &self);
        
        WHEN("Asking for next hop to 888888(shouldn't return one)") {
            sn_addr_t dst;
            sn_entry_t nexthop;

            sn_addr_from_hex(&dst, "888888");

            sn_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 0); //No nexthop, deliver to self
            REQUIRE(sn_addr_cmp(&self, &(nexthop.sn_addr)) == 0);
        }
    }

    GIVEN("A router on 4f5e22 that knows of 888888") {
        sn_router_t r;
        sn_addr_t self;
        sn_addr_t rem;
        
        sn_addr_from_hex(&self, "4f5e22");
        sn_router_init(&r, &self);

        sn_addr_from_hex(&rem, "888888");

        sn_router_add(&r, &rem, NULL);
        
        WHEN("Asking for next hop to 888888") {
            sn_addr_t dst;
            sn_entry_t nexthop;

            sn_addr_from_hex(&dst, "888888");

            sn_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 1);
            REQUIRE(sn_addr_cmp(&rem, &(nexthop.sn_addr)) == 0);
        }

        WHEN("Asking for next hop to 488888") {
            sn_addr_t dst;
            sn_entry_t nexthop;

            sn_addr_from_hex(&dst, "488888");

            sn_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 0); //No nexthop, deliver to self
            REQUIRE(sn_addr_cmp(&self, &(nexthop.sn_addr)) == 0);
        }

        WHEN("Asking for next hop to 788888") {
            sn_addr_t dst;
            sn_entry_t nexthop;

            sn_addr_from_hex(&dst, "788888");

            sn_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 1);
            REQUIRE(sn_addr_cmp(&rem, &(nexthop.sn_addr)) == 0);
        }

        WHEN("Removing 888888 and asking 788888") {
            sn_addr_t dst;
            sn_entry_t nexthop;

            sn_router_remove(&r, &rem);

            sn_addr_from_hex(&dst, "788888");

            sn_router_nexthop(&r, &dst, &nexthop);

            REQUIRE(nexthop.is_set == 0);
            REQUIRE(sn_addr_cmp(&self, &(nexthop.sn_addr)) == 0);
        }
    }
}
