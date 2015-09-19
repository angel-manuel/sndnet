#include "catch.hpp"

#include <address.h>

#include <stdio.h>
#include <string.h>

TEST_CASE("Uppercase initialization", "[address]") {
    SNAddress addr;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";

    sndnet_address_from_hexstr(&addr, hex);

    REQUIRE(strcasecmp(sndnet_address_tostr(&addr), hex) == 0);
    REQUIRE(strcmp(sndnet_address_tostr(&addr), hex_lower) == 0);
}

TEST_CASE("Comparison", "address") {
    SNAddress addr;
    SNAddress addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";

    sndnet_address_from_hexstr(&addr, hex);
    sndnet_address_from_hexstr(&addr2, hex2);

    REQUIRE(sndnet_address_cmp(&addr, &addr2) > 0);
}

TEST_CASE("Distance", "address") {
    SNAddress addr;
    SNAddress addr2;
    SNAddress dist;
    SNAddress exp_dist;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    const char hex_dist[] = "0000000000000000000000000000000000000000000000000000000000040001";

    sndnet_address_from_hexstr(&addr, hex);
    sndnet_address_from_hexstr(&addr2, hex2);
    sndnet_address_from_hexstr(&exp_dist, hex_dist);

    sndnet_address_dist(&addr, &addr2, &dist);

    REQUIRE(sndnet_address_cmp(&dist, &exp_dist) == 0);
}

TEST_CASE("Indexing", "address") {
    SNAddress addr;
    SNAddress addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    unsigned int level = 0;
    unsigned char column = 0;

    sndnet_address_from_hexstr(&addr, hex);
    sndnet_address_from_hexstr(&addr2, hex2);

    sndnet_address_index(&addr, &addr2, &level, &column);
    REQUIRE(level == 59);
    REQUIRE(column == 10);
}

TEST_CASE("Copy", "address") {
    SNAddress addr;
    SNAddress addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";

    sndnet_address_from_hexstr(&addr, hex);
    sndnet_address_copy(&addr2, &addr);

    REQUIRE(sndnet_address_cmp(&addr, &addr2) == 0);
}

TEST_CASE("Short initialization", "address") {
    SNAddress addr;
    const char hex[] = "abcd";
    const char hex_full[] = "abcd000000000000000000000000000000000000000000000000000000000000";

    sndnet_address_from_hexstr(&addr, hex);
    REQUIRE(strcmp(sndnet_address_tostr(&addr), hex_full) == 0);
}