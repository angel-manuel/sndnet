#include "../catch.hpp"

#include <io/naddr.h>

#include <string.h>

TEST_CASE("NAddr from string and to string again", "[io_naddr]") {
    char orig[SN_IO_NADDR_PRINTABLE_LEN] = "INET:6.7.33.4:7777";
    char reco[SN_IO_NADDR_PRINTABLE_LEN];
    sn_io_naddr_t addr;

    REQUIRE(sn_io_naddr_from_str(&addr, orig) == 0);
    REQUIRE(sn_io_naddr_to_str(&addr, reco) == 0);

    REQUIRE(strcmp(orig, reco) == 0);
}
