#include "catch.hpp"

#include <sndnet.h>

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

TEST_CASE("Emulated network #1", "[network]") {
    sn_state_t A, B, C;
    sn_net_addr_t a3f4, b567, b666;
    sn_io_sock_t sockA, sockB, sockC;
    sn_io_naddr_t addrA, addrB, addrC;
    sn_closure_t silent;

    sn_closure_init_curried_once(&silent, sn_silent_log_callback, NULL);

    sn_net_addr_from_hex(&a3f4, "a3f4");
    sn_net_addr_from_hex(&b567, "b567");
    sn_net_addr_from_hex(&b666, "b666");

    sn_io_naddr_local(&addrA, "_A");
    sn_io_naddr_local(&addrB, "_B");
    sn_io_naddr_local(&addrC, "_C");

    REQUIRE((sockA = sn_io_sock_named(&addrA)) != SN_IO_SOCK_INVALID);
    REQUIRE((sockB = sn_io_sock_named(&addrB)) != SN_IO_SOCK_INVALID);
    REQUIRE((sockC = sn_io_sock_named(&addrC)) != SN_IO_SOCK_INVALID);

    REQUIRE(sn_init(&A, &a3f4, sockA) == 0);
    REQUIRE(sn_init(&B, &b567, sockB) == 0);
    REQUIRE(sn_init(&C, &b666, sockC) == 0);
    sn_set_log_callback(&A, &silent);
    sn_set_log_callback(&B, &silent);
    sn_set_log_callback(&C, &silent);

    sn_net_router_add(&A.router, &b567, &addrB);
    sn_net_router_add(&B.router, &a3f4, &addrA);

    sn_net_router_add(&B.router, &b666, &addrC);
    sn_net_router_add(&C.router, &b567, &addrB);

    SECTION("Inserting b667 at A and send to b667 from A") {
        sn_net_addr_t b667;
        sn_io_naddr_t addrTEST;
        sn_io_sock_t sockTEST;

        sn_net_addr_from_hex(&b667, "b667");
        sn_io_naddr_local(&addrTEST, "_TEST");
        REQUIRE((sockTEST = sn_io_sock_named(&addrTEST)) != SN_IO_SOCK_INVALID);

        sn_net_router_add(&A.router, &b667, &addrTEST);

        REQUIRE(sn_send(&A, &b667, 5, "Hola") == 0);

        SECTION("Receiving at b667") {
            sn_net_packet_t* msg;

            msg = sn_net_packet_recv(sockTEST, NULL);

            REQUIRE(msg != NULL);

            REQUIRE(strcmp("Hola", (char*)msg->payload) == 0);

            free(msg);
        }

        sn_io_sock_close(sockTEST);
    }

    SECTION("Inserting b667 at C and send to b667 from A") {
        sn_net_addr_t b667;
        sn_io_naddr_t addrTEST;
        sn_io_sock_t sockTEST;

        sn_net_addr_from_hex(&b667, "b667");
        sn_io_naddr_local(&addrTEST, "_TEST");
        REQUIRE((sockTEST = sn_io_sock_named(&addrTEST)) != SN_IO_SOCK_INVALID);

        sn_net_router_add(&C.router, &b667, &addrTEST);

        REQUIRE(sn_send(&A, &b667, 5, "Hola") == 0);

        SECTION("Receiving at b667") {
            sn_net_packet_t* msg;

            msg = sn_net_packet_recv(sockTEST, NULL);

            REQUIRE(msg != 0);

            REQUIRE(strcmp("Hola", (char*)msg->payload) == 0);

            free(msg);
        }

        sn_io_sock_close(sockTEST);
    }

    sn_destroy(&A);
    sn_destroy(&B);
    sn_destroy(&C);
}
