#include "catch.hpp"

#include <localaddr.h>
#include <netaddr.h>
#include <realaddr.h>

#include <string.h>

TEST_CASE("Netaddress tostr works", "[netaddr]") {
    sn_realaddr_t real;
    sn_localaddr_t local;
    char real_str[SN_NETADDR_PRINTABLE_LEN];
    char local_str[SN_NETADDR_PRINTABLE_LEN];

    REQUIRE(sn_realaddr_from_str(&real, "6.77.22.123:49860") == 0);
    sn_localaddr_init(&local, "lolcat");

    REQUIRE(sn_netaddr_tostr((sn_netaddr_t*)&real, real_str) == 0);
    REQUIRE(sn_netaddr_tostr((sn_netaddr_t*)&local, local_str) == 0);

    REQUIRE(strcmp("INET:6.77.22.123:49860", real_str) == 0);
    REQUIRE(strcmp("UNIX:Hlolcat", local_str) == 0);
}

TEST_CASE("Netaddress comparison works", "[netaddr]") {
    sn_realaddr_t real, real2;
    sn_localaddr_t local, local2;

    REQUIRE(sn_realaddr_from_str(&real, "6.77.22.123:49860") == 0);
    REQUIRE(sn_realaddr_from_str(&real2, "6.77.23.233:49860") == 0);
    sn_localaddr_init(&local, "lolcat");
    sn_localaddr_init(&local2, "lelcat");

    REQUIRE(sn_netaddr_cmp((sn_netaddr_t*)&real, (sn_netaddr_t*)&local) != 0);
    REQUIRE(sn_realaddr_cmp(&real, &real2) < 0);
    REQUIRE(sn_netaddr_cmp((sn_netaddr_t*)&real, (sn_netaddr_t*)&real2) < 0);
    REQUIRE(sn_netaddr_cmp((sn_netaddr_t*)&real, (sn_netaddr_t*)&real) == 0);
    REQUIRE(sn_netaddr_cmp((sn_netaddr_t*)&local, (sn_netaddr_t*)&local) == 0);
    REQUIRE(sn_netaddr_cmp((sn_netaddr_t*)&local, (sn_netaddr_t*)&local2) > 0);
}
