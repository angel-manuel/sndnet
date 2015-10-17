#include "../catch.hpp"

#include <net/addr.h>

#include <stdio.h>
#include <string.h>

TEST_CASE("Address uppercase initialization", "[addr]") {
    sn_net_addr_t addr;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    char ret_str[SN_NET_ADDR_PRINTABLE_LEN];

    sn_net_addr_from_hex(&addr, hex);

    sn_net_addr_to_str(&addr, ret_str);

    REQUIRE(strcasecmp(ret_str, hex) == 0);
    REQUIRE(strcmp(ret_str, hex_lower) == 0);
}

TEST_CASE("Address comparison", "[addr]") {
    sn_net_addr_t addr;
    sn_net_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";

    sn_net_addr_from_hex(&addr, hex);
    sn_net_addr_from_hex(&addr2, hex2);

    REQUIRE(sn_net_addr_cmp(&addr, &addr2) > 0);
}

TEST_CASE("Address distance", "[addr]") {
    sn_net_addr_t addr;
    sn_net_addr_t addr2;
    sn_net_addr_t dist;
    sn_net_addr_t exp_dist;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    const char hex_dist[] = "0000000000000000000000000000000000000000000000000000000000040001";

    sn_net_addr_from_hex(&addr, hex);
    sn_net_addr_from_hex(&addr2, hex2);
    sn_net_addr_from_hex(&exp_dist, hex_dist);

    sn_net_addr_dist(&addr, &addr2, &dist);

    REQUIRE(sn_net_addr_cmp(&dist, &exp_dist) == 0);
}

TEST_CASE("Address indexing", "[addr]") {
    sn_net_addr_t addr;
    sn_net_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";
    const char hex_lower[] = "0000111122223333444455556666777788889999aaaabbbbccccddddeeeeffff";
    const char hex2[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeAfffe";
    unsigned int level = 0;
    unsigned char column = 0;

    sn_net_addr_from_hex(&addr, hex);
    sn_net_addr_from_hex(&addr2, hex2);

    sn_net_addr_index(&addr, &addr2, &level, &column);
    REQUIRE(level == 59);
    REQUIRE(column == 10);
}

TEST_CASE("Address copy", "[addr]") {
    sn_net_addr_t addr;
    sn_net_addr_t addr2;
    const char hex[] = "0000111122223333444455556666777788889999AAAABBBBCCCCddddeeeeffff";

    sn_net_addr_from_hex(&addr, hex);
    addr2 = addr;

    REQUIRE(sn_net_addr_cmp(&addr, &addr2) == 0);
}

TEST_CASE("Address short initialization", "[addr]") {
    sn_net_addr_t addr;
    const char hex[] = "abcd";
    char ret_str[SN_NET_ADDR_PRINTABLE_LEN];

    sn_net_addr_from_hex(&addr, hex);

    sn_net_addr_to_str(&addr, ret_str);

    REQUIRE(strcmp(ret_str, hex) == 0);
}

TEST_CASE("Address serialization and deserialization", "[addr]") {
    sn_net_addr_t orig;
    sn_net_addr_t reco;
    sn_net_addr_ser_t ser;

    sn_net_addr_from_hex(&orig, "fabedef7ca");
    REQUIRE(sn_net_addr_ser(&orig, &ser) == 0);
    REQUIRE(sn_net_addr_deser(&reco, &ser) == 0);

    REQUIRE(sn_net_addr_cmp(&orig, &reco) == 0);
}
