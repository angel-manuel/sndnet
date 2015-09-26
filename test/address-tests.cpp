#include "catch.hpp"

#include <address.h>

#include <stdio.h>
#include <string.h>

TEST_CASE("Address uppercase initialization", "[address]") {
    sndnet_addr_t addr;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    char ret_str[SNDNET_ADDRESS_PRINTABLE_LENGTH];

    sndnet_address_from_hex(&addr, hex);

    sndnet_address_tostr(&addr, ret_str);

    REQUIRE(strcasecmp(ret_str, hex) == 0);
    REQUIRE(strcmp(ret_str, hex_lower) == 0);
}

TEST_CASE("Address comparison", "address") {
    sndnet_addr_t addr;
    sndnet_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";

    sndnet_address_from_hex(&addr, hex);
    sndnet_address_from_hex(&addr2, hex2);

    REQUIRE(sndnet_address_cmp(&addr, &addr2) > 0);
}

TEST_CASE("Address distance", "address") {
    sndnet_addr_t addr;
    sndnet_addr_t addr2;
    sndnet_addr_t dist;
    sndnet_addr_t exp_dist;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    const char hex_dist[] = "0000000000000000000000000000000000000000000000000000000000040001";

    sndnet_address_from_hex(&addr, hex);
    sndnet_address_from_hex(&addr2, hex2);
    sndnet_address_from_hex(&exp_dist, hex_dist);

    sndnet_address_dist(&addr, &addr2, &dist);

    REQUIRE(sndnet_address_cmp(&dist, &exp_dist) == 0);
}

TEST_CASE("Address indexing", "address") {
    sndnet_addr_t addr;
    sndnet_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    unsigned int level = 0;
    unsigned char column = 0;

    sndnet_address_from_hex(&addr, hex);
    sndnet_address_from_hex(&addr2, hex2);

    sndnet_address_index(&addr, &addr2, &level, &column);
    REQUIRE(level == 59);
    REQUIRE(column == 10);
}

TEST_CASE("Address copy", "address") {
    sndnet_addr_t addr;
    sndnet_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";

    sndnet_address_from_hex(&addr, hex);
    addr2 = addr;

    REQUIRE(sndnet_address_cmp(&addr, &addr2) == 0);
}

TEST_CASE("Address short initialization", "address") {
    sndnet_addr_t addr;
    const char hex[] = "abcd";
    const char hex_full[] = "abcd000000000000000000000000000000000000000000000000000000000000";
    char ret_str[SNDNET_ADDRESS_PRINTABLE_LENGTH];

    sndnet_address_from_hex(&addr, hex);

    sndnet_address_tostr(&addr, ret_str);

    REQUIRE(strcmp(ret_str, hex_full) == 0);
}