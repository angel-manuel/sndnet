#include "../catch.hpp"

#include "data/vec.h"

TEST_CASE("data/vec: Pushing and reading from capacity 0", "[data_vec]") {
    sn_data_vec_t v;
    int i25 = 25, i10 = 10, i42 = 42;
    int iout = 0;

    REQUIRE(sn_data_vec_with_capacity(&v, sizeof(int), 0) == 0);

    REQUIRE(sn_data_vec_push(&v, &i25) == 0); 
    REQUIRE(sn_data_vec_push(&v, &i42) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);

    //v = [25, 42, 10]

    REQUIRE(sn_data_vec_remove_at(&v, 1, &iout) == 0);
    REQUIRE(iout == 42);

    //v = [25, 10]

    REQUIRE(sn_data_vec_at(&v, 1, &iout) == 0);
    REQUIRE(iout == 10);

    REQUIRE(sn_data_vec_at(&v, 0, &iout) == 0);
    REQUIRE(iout == 25);

    REQUIRE(sn_data_vec_at(&v, 5, &iout) == -1);
    
    REQUIRE(sn_data_vec_remove_at(&v, 0, NULL) == 0);

    //v = [10]

    REQUIRE(sn_data_vec_at(&v, 0, &iout) == 0);
    REQUIRE(iout == 10);

    sn_data_vec_destroy(&v);
}

TEST_CASE("data/vec: Pushing and reading from capacity 8", "[data_vec]") {
    sn_data_vec_t v;
    int i25 = 25, i10 = 10, i42 = 42;
    int iout = 0;

    REQUIRE(sn_data_vec_with_capacity(&v, sizeof(int), 8) == 0);

    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);

    REQUIRE(sn_data_vec_push(&v, &i25) == 0); 
    REQUIRE(sn_data_vec_push(&v, &i42) == 0);
    REQUIRE(sn_data_vec_push(&v, &i10) == 0);

    //v+8 = [25, 42, 10]

    REQUIRE(sn_data_vec_remove_at(&v, 8+1, &iout) == 0);
    REQUIRE(iout == 42);

    //v+8 = [25, 10]

    REQUIRE(sn_data_vec_at(&v, 8+1, &iout) == 0);
    REQUIRE(iout == 10);

    REQUIRE(sn_data_vec_at(&v, 8+0, &iout) == 0);
    REQUIRE(iout == 25);

    REQUIRE(sn_data_vec_at(&v, 8+5, &iout) == -1);
    REQUIRE(sn_data_vec_at(&v, 8+2, &iout) == -1);
    
    REQUIRE(sn_data_vec_remove_at(&v, 8+0, NULL) == 0);

    //v+8 = [10]

    REQUIRE(sn_data_vec_at(&v, 8+0, &iout) == 0);
    REQUIRE(iout == 10);

    REQUIRE(sn_data_vec_remove_at(&v, 8+0, NULL) == 0);
    REQUIRE(v.capacity == 8);

    sn_data_vec_destroy(&v);
}
