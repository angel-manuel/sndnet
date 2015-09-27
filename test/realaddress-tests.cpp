#include "catch.hpp"

#include <realaddress.h>

#include <string.h>

TEST_CASE("Realaddress creation and recovery", "[realaddress]") {
    sndnet_realaddr_t snra;
    const char* orig_name = "222.123.54.3";
    char recovered_name[SNDNET_REALADDRESS_HOSTNAME_PRINTABLE_LENGTH] = "\0";
    uint16_t recovered_port = 0;

    REQUIRE(sndnet_realaddress_from_hostname(&snra, orig_name, 7777) == 0);

    REQUIRE(sndnet_realaddress_get_hostname(&snra, recovered_name) == 0);

    REQUIRE(strcmp(orig_name, recovered_name) == 0);

    REQUIRE(sndnet_realaddress_get_port(&snra, &recovered_port) == 0);

    REQUIRE(recovered_port == 7777);
}

TEST_CASE("Realaddress creation from string and recovery", "[realaddress]") {
    sndnet_realaddr_t snra;
    const char* orig_name = "222.123.54.3";
    char recovered_name[SNDNET_REALADDRESS_HOSTNAME_PRINTABLE_LENGTH] = "\0";
    uint16_t recovered_port = 0;

    REQUIRE(sndnet_realaddress_from_str(&snra, "222.123.54.3:7777") == 0);

    REQUIRE(sndnet_realaddress_get_hostname(&snra, recovered_name) == 0);

    REQUIRE(strcmp(orig_name, recovered_name) == 0);

    REQUIRE(sndnet_realaddress_get_port(&snra, &recovered_port) == 0);

    REQUIRE(recovered_port == 7777);
}

