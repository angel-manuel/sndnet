#include "catch.hpp"

#include <sndnet.h>

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

TEST_CASE("Emulated network #1", "[network]") {
    sn_state_t A, B, C;
    sn_addr_t a3f4, b567, b666;
    sn_sock_t sockA, sockB, sockC;
    sn_netaddr_t addrA, addrB, addrC;

    sn_addr_from_hex(&a3f4, "a3f4");
    sn_addr_from_hex(&b567, "b567");
    sn_addr_from_hex(&b666, "b666");

    sn_localaddr_init((sn_localaddr_t*)&addrA, "A");
    sn_localaddr_init((sn_localaddr_t*)&addrB, "B");
    sn_localaddr_init((sn_localaddr_t*)&addrC, "C");

    REQUIRE(sn_sock_init_binded(&sockA, &addrA) == 0);
    REQUIRE(sn_sock_init_binded(&sockB, &addrB) == 0);
    REQUIRE(sn_sock_init_binded(&sockC, &addrC) == 0);

    REQUIRE(sn_init(&A, &a3f4, &sockA) == 0);
    REQUIRE(sn_init(&B, &b567, &sockB) == 0);
    REQUIRE(sn_init(&C, &b666, &sockC) == 0);
    sn_set_log_callback(&A, sn_silent_log_callback, NULL);
    sn_set_log_callback(&B, sn_silent_log_callback, NULL);
    sn_set_log_callback(&C, sn_silent_log_callback, NULL);

    sn_router_add(&A.router, &b567, &addrB);
    sn_router_add(&B.router, &a3f4, &addrA);

    sn_router_add(&B.router, &b666, &addrC);
    sn_router_add(&C.router, &b567, &addrB);

    SECTION("Inserting b667 at A and send to b667 from A") {
        sn_addr_t b667;
        sn_netaddr_t addrTEST;
        sn_sock_t sockTEST;

        sn_addr_from_hex(&b667, "b667");
        sn_localaddr_init((sn_localaddr_t*)&addrTEST, "TEST");
        REQUIRE(sn_sock_init_binded(&sockTEST, &addrTEST) == 0);

        sn_router_add(&A.router, &b667, &addrTEST);

        REQUIRE(sn_send(&A, &b667, 5, "Hola") == 0);

        /* sn_send(&A, &b667, 5, "Hola");
        printf("err = %s\n", strerror(errno)); */

        SECTION("Receiving at b667") {
            sn_msg_t* msg;

            msg = sn_msg_recv(&sockTEST, 0);

            REQUIRE(msg != 0);

            REQUIRE(strcmp("Hola", (char*)msg->payload) == 0);

            free(msg);
        }

        sn_sock_destroy(&sockTEST);
    }

    SECTION("Inserting b667 at C and send to b667 from A") {
        sn_addr_t b667;
        sn_netaddr_t addrTEST;
        sn_sock_t sockTEST;

        sn_addr_from_hex(&b667, "b667");
        sn_localaddr_init((sn_localaddr_t*)&addrTEST, "TEST");
        REQUIRE(sn_sock_init_binded(&sockTEST, &addrTEST) == 0);

        sn_router_add(&C.router, &b667, &addrTEST);

        REQUIRE(sn_send(&A, &b667, 5, "Hola") == 0);

        SECTION("Receiving at b667") {
            sn_msg_t* msg;

            msg = sn_msg_recv(&sockTEST, 0);

            REQUIRE(msg != 0);

            REQUIRE(strcmp("Hola", (char*)msg->payload) == 0);

            free(msg);
        }

        sn_sock_destroy(&sockTEST);
    }

    sn_destroy(&A);
    sn_destroy(&B);
    sn_destroy(&C);
}
