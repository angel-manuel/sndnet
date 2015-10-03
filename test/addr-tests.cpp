#include "catch.hpp"

#include <addr.h>

#include <stdio.h>
#include <string.h>

TEST_CASE("Address uppercase initialization", "[addr]") {
    sn_addr_t addr;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    char ret_str[SN_ADDR_PRINTABLE_LEN];

    sn_addr_from_hex(&addr, hex);

    sn_addr_tostr(&addr, ret_str);

    REQUIRE(strcasecmp(ret_str, hex) == 0);
    REQUIRE(strcmp(ret_str, hex_lower) == 0);
}

TEST_CASE("Address comparison", "[addr]") {
    sn_addr_t addr;
    sn_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";

    sn_addr_from_hex(&addr, hex);
    sn_addr_from_hex(&addr2, hex2);

    REQUIRE(sn_addr_cmp(&addr, &addr2) > 0);
}

TEST_CASE("Address distance", "[addr]") {
    sn_addr_t addr;
    sn_addr_t addr2;
    sn_addr_t dist;
    sn_addr_t exp_dist;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    const char hex_dist[] = "0000000000000000000000000000000000000000000000000000000000040001";

    sn_addr_from_hex(&addr, hex);
    sn_addr_from_hex(&addr2, hex2);
    sn_addr_from_hex(&exp_dist, hex_dist);

    sn_addr_dist(&addr, &addr2, &dist);

    REQUIRE(sn_addr_cmp(&dist, &exp_dist) == 0);
}

TEST_CASE("Address indexing", "[addr]") {
    sn_addr_t addr;
    sn_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    unsigned int level = 0;
    unsigned char column = 0;

    sn_addr_from_hex(&addr, hex);
    sn_addr_from_hex(&addr2, hex2);

    sn_addr_index(&addr, &addr2, &level, &column);
    REQUIRE(level == 59);
    REQUIRE(column == 10);
}

TEST_CASE("Address copy", "[addr]") {
    sn_addr_t addr;
    sn_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";

    sn_addr_from_hex(&addr, hex);
    addr2 = addr;

    REQUIRE(sn_addr_cmp(&addr, &addr2) == 0);
}

TEST_CASE("Address short initialization", "[addr]") {
    sn_addr_t addr;
    const char hex[] = "abcd";
    const char hex_full[] = "abcd000000000000000000000000000000000000000000000000000000000000";
    char ret_str[SN_ADDR_PRINTABLE_LEN];

    sn_addr_from_hex(&addr, hex);

    sn_addr_tostr(&addr, ret_str);

    REQUIRE(strcmp(ret_str, hex_full) == 0);
}
