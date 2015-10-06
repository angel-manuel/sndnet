#include "catch.hpp"

#include <router.h>

#include <stdio.h>

SCENARIO("routing is correct", "[router]") {
    GIVEN("An empty router on 4f5e22") {
        sn_router_t r;
        sn_addr_t self;

        sn_addr_from_hex(&self, "4f5e22");
        sn_router_init(&r, &self, NULL);

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
        sn_router_init(&r, &self, NULL);

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

SCENARIO("inserting into router", "[router]") {
    GIVEN("An empty router on 1234") {
        sn_router_t r;
        sn_addr_t self;

        sn_addr_from_hex(&self, "1234");
        sn_router_init(&r, &self, NULL);

        REQUIRE(sn_addr_cmp(&sn_router_leafset_get(&r, 0)->sn_addr, &self) == 0);

        WHEN("Adding 3333 it appears on both leafset and table") {
            sn_addr_t addr3333;
            const sn_entry_t* res;

            sn_addr_from_hex(&addr3333, "3333");

            sn_router_add(&r, &addr3333, NULL);

            res = sn_router_leafset_get(&r, 1);

            REQUIRE(sn_addr_cmp(&res->sn_addr, &addr3333) == 0);

            res = sn_router_table_get(&r, 0, 3);

            REQUIRE(sn_addr_cmp(&res->sn_addr, &addr3333) == 0);
        }
    }

    GIVEN("A router on 1234 with the right leafset filled with 400x addresses") {
        sn_router_t r;
        sn_addr_t self;
        int i;

        sn_addr_from_hex(&self, "1234");
        sn_router_init(&r, &self, NULL);

        for(i = 0; i < SN_ROUTER_LEAFSET_SIZE; ++i) {
            sn_addr_t insert;
            char baseaddr[10];

            snprintf(baseaddr, 10, "%d", 4000 + i);
            sn_addr_from_hex(&insert, baseaddr);

            sn_router_add(&r, &insert, NULL);
        }

        WHEN("Adding 3333 it appears on both leafset and table") {
            sn_addr_t addr3333;
            const sn_entry_t* res;

            sn_addr_from_hex(&addr3333, "3333");

            sn_router_add(&r, &addr3333, NULL);

            res = sn_router_leafset_get(&r, 1);

            REQUIRE(sn_addr_cmp(&res->sn_addr, &addr3333) == 0);

            res = sn_router_table_get(&r, 0, 3);

            REQUIRE(sn_addr_cmp(&res->sn_addr, &addr3333) == 0);
        }
    }
}

TEST_CASE("Router overwriting", "[router]") {
    sn_router_t r;
    sn_entry_t e;
    sn_entry_t self;

    self.is_set = 1;
    sn_addr_from_hex(&self.sn_addr, "0a0a0a0a");
    sn_realaddr_from_str(&self.net_addr, "1.1.1.1:1111");

    sn_router_init(&r, &self.sn_addr, &self.net_addr);

    REQUIRE(sn_entry_cmp(sn_router_leafset_get(&r, 0), &self) == 0);

    e.is_set = 1;
    sn_addr_from_hex(&e.sn_addr, "abcdef");
    sn_realaddr_from_str(&e.net_addr, "5.6.7.8:8765");

    sn_router_table_set(&r, 1, 2, &e);

    REQUIRE(sn_entry_cmp(sn_router_table_get(&r, 1, 2), &e) == 0);

    sn_router_leafset_set(&r, -1, &e);

    REQUIRE(sn_entry_cmp(sn_router_leafset_get(&r, -1), &e) == 0);
}

TEST_CASE("Router querying", "[router]") {
    sn_router_t r;
    sn_entry_t e;
    sn_entry_t self;
    sn_entry_t reco;
    sn_router_query_ser_t* query_res;

    self.is_set = 1;
    sn_addr_from_hex(&self.sn_addr, "0a0a0a0a");
    sn_realaddr_from_str(&self.net_addr, "1.1.1.1:1111");

    sn_router_init(&r, &self.sn_addr, &self.net_addr);

    e.is_set = 1;
    sn_addr_from_hex(&e.sn_addr, "abcdef");
    sn_realaddr_from_str(&e.net_addr, "5.6.7.8:8765");

    sn_router_table_set(&r, 1, 2, &e);
    sn_router_table_set(&r, 1, 4, &e);
    sn_router_table_set(&r, 1, 6, &e);

    sn_router_leafset_set(&r, -1, &e);
    sn_router_leafset_set(&r, -5, &e);
    sn_router_leafset_set(&r, +3, &e);

    REQUIRE(sn_router_query_table(&r, 1, 5, &query_res));

    REQUIRE(query_res->entries_len == 3);

    REQUIRE(query_res->entries[0].is_table == 1);
    REQUIRE(query_res->entries[0].level == 1);
    REQUIRE(query_res->entries[0].column == 2);

    REQUIRE(sn_entry_deser(&reco, &query_res->entries[0].entry) == 0);

    REQUIRE(sn_entry_equals(&reco, &e));

    free(query_res);

    REQUIRE(sn_router_query_leafset(&r, -3, +3, &query_res));

    REQUIRE(query_res->entries_len == 3);

    REQUIRE(query_res->entries[0].is_table == 0);
    REQUIRE(query_res->entries[0].position == -1);

    REQUIRE(sn_entry_deser(&reco, &query_res->entries[0].entry) == 0);

    REQUIRE(sn_entry_equals(&reco, &e));

    free(query_res);
}
