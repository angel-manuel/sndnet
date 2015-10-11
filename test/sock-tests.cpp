#include "catch.hpp"

#include <sock.h>
#include <realaddr.h>
#include <localaddr.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

TEST_CASE("Socket over realaddress works", "[sock]") {
    sn_realaddr_t addr_local, addr_rem;
    sn_sock_t sockA, sockB;
    char buffer[5];

    REQUIRE(sn_realaddr_local_at_port(&addr_local, 7777) == 0);
    REQUIRE(sn_realaddr_from_hostname(&addr_rem, "127.0.0.1", 7777) == 0);

    REQUIRE(sn_sock_init(&sockA, AF_INET) == 0);
    REQUIRE(sn_sock_init_binded(&sockB, (sn_netaddr_t*)&addr_local) == 0);

    REQUIRE(sn_sock_send(&sockA, "Hola", 5, 0, &addr_rem) == 5);

    REQUIRE(sn_sock_recv(&sockB, buffer, 5, 0, 0) == 5);

    REQUIRE(strcmp(buffer, "Hola") == 0);

    sn_sock_destroy(&sockA);
    sn_sock_destroy(&sockB);
}

TEST_CASE("Socket over localaddress works", "[sock]") {
    sn_localaddr_t addr;
    sn_sock_t sockA, sockB;
    char buffer[5];

    sn_localaddr_init(&addr, "TEST");

    REQUIRE(sn_sock_init(&sockA, AF_UNIX) == 0);
    REQUIRE(sn_sock_init_binded(&sockB, (sn_netaddr_t*)&addr) == 0);

    REQUIRE(sn_sock_send(&sockA, "Hola", 5, 0, (sn_netaddr_t*)&addr) == 5);

    REQUIRE(sn_sock_recv(&sockB, buffer, 5, 0, 0) == 5);

    REQUIRE(strcmp(buffer, "Hola") == 0);

    sn_sock_destroy(&sockA);
    sn_sock_destroy(&sockB);
}
