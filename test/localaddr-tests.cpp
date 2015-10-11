#include "catch.hpp"

#include <localaddr.h>

#include <string.h>

TEST_CASE("Localaddress creation and recovery", "[localaddr]") {
    sn_localaddr_t la;
    char la_str[SN_LOCALADDR_PRINTABLE_LEN];

    sn_localaddr_init(&la, "endpoint");

    sn_localaddr_tostr(&la, la_str);

    #ifdef SN_LOCALADDR_HIDDEN
    REQUIRE(strcmp("_endpoint", la_str) == 0);
    #else
    REQUIRE(strcmp("endpoint", la_str) == 0);
    #endif
}

TEST_CASE("Localaddress comparison works", "[localaddr]") {
    sn_localaddr_t a, b;

    sn_localaddr_init(&a, "aaaaaa");
    sn_localaddr_init(&b, "bbbbbb");

    REQUIRE(sn_localaddr_cmp(&a, &b) < 0);
}

TEST_CASE("Localaddress serialization and deserialization works", "[localaddr]") {
    sn_localaddr_t orig;
    sn_localaddr_t reco;
    sn_localaddr_ser_t ser;

    sn_localaddr_init(&orig, "endpoint");
    REQUIRE(sn_localaddr_ser(&orig, &ser) == 0);
    REQUIRE(sn_localaddr_deser(&reco, &ser) == 0);

    REQUIRE(sn_localaddr_cmp(&orig, &reco) == 0);
}
