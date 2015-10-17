#include "../catch.hpp"

#include <net/addr.h>
#include <net/entry.h>

TEST_CASE("Entry comparison", "[entry]") {
	sn_net_entry_t e1, e2;

	sn_net_addr_from_hex(&e1.sn_net_addr, "1234");
	sn_net_addr_from_hex(&e2.sn_net_addr, "e453");

	REQUIRE(sn_net_entry_cmp(&e1, &e2) < 0);
	REQUIRE(sn_net_entry_cmp_neg(&e1, &e2) > 0);
}

TEST_CASE("Entry array length", "[entry]") {
	sn_net_entry_t arr[10];
	int i;

	for(i = 0; i < 10; ++i)
		arr[i].is_set = (i < 5);

	REQUIRE(sn_net_entry_array_len(arr, 10) == 5);
}

TEST_CASE("Entry closest", "[entry]") {
	sn_net_entry_t cand[5];
	sn_net_addr_t self;
	sn_net_addr_t dst;
	sn_net_entry_t res;
	int i;

	for(i = 0; i < 5; ++i)
		cand[i].is_set = 1;

	sn_net_addr_from_hex(&cand[0].sn_net_addr, "10000");
	sn_net_addr_from_hex(&cand[1].sn_net_addr, "11000");
	sn_net_addr_from_hex(&cand[2].sn_net_addr, "12000");
	sn_net_addr_from_hex(&cand[3].sn_net_addr, "13000");
	sn_net_addr_from_hex(&cand[4].sn_net_addr, "14000");

	sn_net_addr_from_hex(&self, "1ffff");
	sn_net_addr_from_hex(&dst, "12345");

	sn_net_entry_closest(&dst, cand, 5, &self, 1, &res);

	REQUIRE(res.is_set);
	REQUIRE(sn_net_addr_cmp(&res.sn_net_addr, &cand[2].sn_net_addr) == 0);
}

TEST_CASE("Entry serialization and deserialization", "[entry]") {
	sn_net_entry_t orig;
	sn_net_entry_t reco;
	sn_net_entry_ser_t ser;

	orig.is_set = 1;
	sn_net_addr_from_hex(&orig.sn_net_addr, "b16b002");
	sn_io_naddr_from_str(&orig.net_addr, "INET:9.8.7.6:54321");

	REQUIRE(sn_net_entry_ser(&orig, &ser) == 0);
	REQUIRE(sn_net_entry_deser(&reco, &ser) == 0);

	REQUIRE(sn_net_entry_equals(&orig, &reco));
}
