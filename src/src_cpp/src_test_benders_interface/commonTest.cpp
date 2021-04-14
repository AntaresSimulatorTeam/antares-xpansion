#include "catch2.hpp"
#include <iostream>
#include "common.h"

TEST_CASE("Test common.h header"){
    
    BendersData data;
    data.it = 5;
    REQUIRE(data.it == 5);

}