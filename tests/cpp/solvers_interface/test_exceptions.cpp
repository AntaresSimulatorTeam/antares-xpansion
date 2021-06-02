#pragma once
#include "catch2.hpp"
#include "multisolver_interface/Solver.h"


TEST_CASE("InvalidStatusException", "[exceptions]") {

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


