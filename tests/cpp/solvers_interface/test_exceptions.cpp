#pragma once
#include "catch2.hpp"
#include "multisolver_interface/Solver.h"


TEST_CASE("InvalidStatusException", "[exceptions][invalid_status]") {

    SolverFactory factory;
    for (auto const& solver_name : factory.get_solvers_list()) {

        //========================================================================================
        // solver declaration
        SolverAbstract::Ptr solver = factory.create_solver(solver_name);
        try {
            solver->zero_status_check(-1, "test exception");
        }
        catch(InvalidStatusException& ex)
        {
            REQUIRE(std::string(ex.what()) == "Failed to test exception: invalid status -1 (0 expected)");
        }
    }
}

TEST_CASE("InvalidRowSizeException", "[exceptions][invalid_row_size]") {

    SolverFactory factory;
    std::string  solver_name = "CBC";
    //========================================================================================
    // solver declaration
    SolverAbstract::Ptr solver = factory.create_solver(solver_name);
    solver->init();

    try{
        std::vector<std::string> names = solver->get_row_names(0,9);
    }catch(InvalidRowSizeException& ex)
    {
        REQUIRE(std::string(ex.what()) == "Invalid row size for solver. 0 rows available (10 expected)");
    }
}

TEST_CASE("InvalidColSizeException", "[exceptions][invalid_col_size]") {

    SolverFactory factory;
    std::string  solver_name = "CBC";
    //========================================================================================
    // solver declaration
    SolverAbstract::Ptr solver = factory.create_solver(solver_name);
    solver->init();


    try{
        std::vector<std::string> names = solver->get_col_names(0,9);
    }catch(InvalidColSizeException& ex)
    {
        REQUIRE(std::string(ex.what()) == "Invalid col size for solver. 0 cols available (10 expected)");
    }
}




