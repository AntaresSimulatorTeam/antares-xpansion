#include "catch2.hpp"
#include <iostream>
#include <chrono>
#include <thread>

#include "Timer.h"

TEST_CASE("Test Timer.h"){
    Timer timer;
    int tps = 1;
    double precision = 0.05;
    std::chrono::seconds dura( tps);
    std::this_thread::sleep_for( dura );
    REQUIRE( timer.elapsed() > tps - precision);
    REQUIRE(timer.elapsed() < tps + precision);
    timer.restart();
    REQUIRE(timer.elapsed() < precision);
}