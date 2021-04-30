#include "catch2.hpp"
#include <iostream>
#include <chrono>
#include <thread>

#include "BendersOptions.h"

TEST_CASE("Test BendersOptions", "[options]"){
    
    BendersOptions options;
    
    //========================================================================================
    // constructor and call to macro
    REQUIRE(options.LOG_LEVEL == 3);
    REQUIRE(options.MAX_ITERATIONS == -1);
    REQUIRE(options.GAP == 1e-6);
    REQUIRE(options.AGGREGATION == false);
    REQUIRE(options.OUTPUTROOT == ".");
    REQUIRE(options.TRACE == true);
    REQUIRE(options.DELETE_CUT == false);
    REQUIRE(options.SLAVE_WEIGHT == "CONSTANT");
    REQUIRE(options.SLAVE_WEIGHT_VALUE == 1);
    REQUIRE(options.MASTER_NAME == "master");
    REQUIRE(options.SLAVE_NUMBER == -1);
    REQUIRE(options.STRUCTURE_FILE == "structure.txt");
    REQUIRE(options.INPUTROOT == ".");
    REQUIRE(options.BASIS == true);
    REQUIRE(options.ACTIVECUTS == false);
    REQUIRE(options.THRESHOLD_AGGREGATION == 0);
    REQUIRE(options.THRESHOLD_ITERATION == 0);
    REQUIRE(options.RAND_AGGREGATION == 0);
    REQUIRE(options.CSV_NAME == "benders_output_trace");
    REQUIRE(options.BOUND_ALPHA == true);
    
    //========================================================================================
    // get_master_path
#if defined(WIN32) || defined(_WIN32)
    REQUIRE(options.get_master_path() == ".\\master.mps");
#else
    REQUIRE(options.get_master_path() == "./master.mps");
#endif
    
    //========================================================================================
    // get_structure_path
#if defined(WIN32) || defined(_WIN32)
    REQUIRE(options.get_structure_path() == ".\\structure.txt");
#else
    REQUIRE(options.get_structure_path() == "./structure.txt");
#endif
    
    //========================================================================================
    // get_slave_path
    const std::string slave_name = "toto";
#if defined(WIN32) || defined(_WIN32)
    REQUIRE(options.get_slave_path(slave_name) == ".\\toto.mps");
#else
    REQUIRE(options.get_slave_path(slave_name) == "./toto.mps");
#endif
    
    //========================================================================================
    // read option file
    options.read("../../data_test/options_file/options_test.txt");

    REQUIRE(options.LOG_LEVEL == 1);
    REQUIRE(options.MAX_ITERATIONS == -1);
    REQUIRE(options.GAP == 1e-1);
    REQUIRE(options.AGGREGATION == true);
    REQUIRE(options.TRACE == false);
    REQUIRE(options.SLAVE_WEIGHT== "UNIFORM");
    REQUIRE(options.MASTER_NAME == "titi");
    REQUIRE(options.SLAVE_NUMBER == 8);   

    //========================================================================================
    // slave_weight
    options.SLAVE_WEIGHT = "UNIFORM";
    options.SLAVE_NUMBER = 50;
    REQUIRE(options.slave_weight(options.SLAVE_NUMBER, options.SLAVE_WEIGHT) == 1.0 / 50.0);
}