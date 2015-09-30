#include "catch.hpp"

#include <address.h>
#include <entry.h>

TEST_CASE("Entry comparison", "[entry]") {
	sn_entry_t e1, e2;
	
	sn_addr_from_hex(&e1.sn_addr, "1234");
	sn_addr_from_hex(&e2.sn_addr, "e453");
	
	REQUIRE(sn_entry_cmp(&e1, &e2) < 0);
	REQUIRE(sn_entry_cmp_neg(&e1, &e2) > 0);
}

TEST_CASE("Entry array length", "[entry]") {
	sn_entry_t arr[10];
	int i;
	
	for(i = 0; i < 10; ++i)
		arr[i].is_set = (i < 5);
	
	REQUIRE(sn_entry_array_len(arr) == 5);
}

TEST_CASE("Entry closest", "[entry]") {
	sn_entry_t cand[5];
	sn_addr_t self;
	sn_addr_t dst;
	sn_entry_t res;
	int i;
	
	for(i = 0; i < 5; ++i)
		cand[i].is_set = 1;
	
	sn_addr_from_hex(&cand[0].sn_addr, "10000");
	sn_addr_from_hex(&cand[1].sn_addr, "11000");
	sn_addr_from_hex(&cand[2].sn_addr, "12000");
	sn_addr_from_hex(&cand[3].sn_addr, "13000");
	sn_addr_from_hex(&cand[4].sn_addr, "14000");
	
	sn_addr_from_hex(&self, "1ffff");
	sn_addr_from_hex(&dst, "12345");
	
	sn_entry_closest(&dst, cand, 5, &self, 1, &res);
	
	REQUIRE(res.is_set);
	REQUIRE(sn_addr_cmp(&res.sn_addr, &cand[2].sn_addr) == 0);
}