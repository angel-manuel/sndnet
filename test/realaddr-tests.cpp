#include "catch.hpp"

#include <realaddr.h>

#include <string.h>

TEST_CASE("Realaddress creation and recovery", "[realaddr]") {
    sn_realaddr_t snra;
    const char* orig_name = "222.123.54.3";
    char recovered_name[SN_REALADDR_HOSTNAME_PRINTABLE_LEN] = "\0";
    uint16_t recovered_port = 0;

    REQUIRE(sn_realaddr_from_hostname(&snra, orig_name, 7777) == 0);

    REQUIRE(sn_realaddr_get_hostname(&snra, recovered_name) == 0);

    REQUIRE(strcmp(orig_name, recovered_name) == 0);

    REQUIRE(sn_realaddr_get_port(&snra, &recovered_port) == 0);

    REQUIRE(recovered_port == 7777);
}

TEST_CASE("Realaddress creation from string and recovery", "[realaddr]") {
    sn_realaddr_t snra;
    const char* orig_name = "222.123.54.3";
    char recovered_name[SN_REALADDR_HOSTNAME_PRINTABLE_LEN] = "\0";
    uint16_t recovered_port = 0;

    REQUIRE(sn_realaddr_from_str(&snra, "222.123.54.3:7777") == 0);

    REQUIRE(sn_realaddr_get_hostname(&snra, recovered_name) == 0);

    REQUIRE(strcmp(orig_name, recovered_name) == 0);

    REQUIRE(sn_realaddr_get_port(&snra, &recovered_port) == 0);

    REQUIRE(recovered_port == 7777);
}

TEST_CASE("Realaddress serialization and deserialization", "[realaddr]") {
    sn_realaddr_t orig;
    sn_realaddr_t reco;
    sn_realaddr_ser_t ser;

    sn_realaddr_from_str(&orig, "1.2.3.4:5678");
    REQUIRE(sn_realaddr_ser(&orig, &ser) == 0);
    REQUIRE(sn_realaddr_deser(&reco, &ser) == 0);

    REQUIRE(sn_realaddr_cmp(&orig, &reco) == 0);
}
