#include "catch.hpp"

#include <address.h>
#include <entry.h>

TEST_CASE("Entry comparison", "[entry]") {
	sndnet_entry_t e1, e2;
	
	sndnet_address_from_hex(&e1.sn_addr, "1234");
	sndnet_address_from_hex(&e2.sn_addr, "e453");
	
	REQUIRE(sndnet_entry_cmp(&e1, &e2) < 0);
	REQUIRE(sndnet_entry_cmp_neg(&e1, &e2) > 0);
}

TEST_CASE("Entry array length", "[entry]") {
	sndnet_entry_t arr[10];
	int i;
	
	for(i = 0; i < 10; ++i)
		arr[i].is_set = (i < 5);
	
	REQUIRE(sndnet_entry_array_len(arr) == 5);
}

TEST_CASE("Entry closest", "[entry]") {
	sndnet_entry_t cand[5];
	sndnet_addr_t self;
	sndnet_addr_t dst;
	sndnet_entry_t res;
	int i;
	
	for(i = 0; i < 5; ++i)
		cand[i].is_set = 1;
	
	sndnet_address_from_hex(&cand[0].sn_addr, "10000");
	sndnet_address_from_hex(&cand[1].sn_addr, "11000");
	sndnet_address_from_hex(&cand[2].sn_addr, "12000");
	sndnet_address_from_hex(&cand[3].sn_addr, "13000");
	sndnet_address_from_hex(&cand[4].sn_addr, "14000");
	
	sndnet_address_from_hex(&self, "1ffff");
	sndnet_address_from_hex(&dst, "12345");
	
	sndnet_entry_closest(&dst, cand, 5, &self, 1, &res);
	
	REQUIRE(res.is_set);
	REQUIRE(sndnet_address_cmp(&res.sn_addr, &cand[2].sn_addr) == 0);
}