#include <iostream>

#include "catch2.hpp"
#include "define_datas.hpp"
#include "multisolver_interface/Solver.h"

TEST_CASE("A LP problem is solved", "[solve-lp]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MULTIKP);
  SECTION("Loop on the instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;

      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance, false);

      //========================================================================================
      // Solve as LP
      int slv_status = solver->solve_lp();

      bool success = false;
      for (auto stat : datas[inst]._status_int) {
        if (stat == slv_status) {
          SUCCEED();
          success = true;
          break;
        }
      }
      if (!success) {
        FAIL();
      }

      success = false;
      for (auto stat : datas[inst]._status) {
        if (stat == solver->SOLVER_STRING_STATUS[slv_status]) {
          SUCCEED();
          success = true;
          break;
        }
      }
      if (!success) {
        FAIL();
      }
      solver->free();
      REQUIRE(solver->get_number_of_instances() == 1);
    }
  }
}

TEST_CASE("A LP problem is solved and we can get the LP value",
          "[solve-lp][get-lp-val]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MULTIKP, NET_MASTER, NET_SP1, NET_SP2);
  SECTION("Loop on the instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      auto instance = datas[inst]._path;
      //========================================================================================
      // Solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance, false);

      //========================================================================================
      // Solve as LP
      int slv_status = solver->solve_lp();

      bool success = false;
      for (auto stat : datas[inst]._status_int) {
        if (stat == slv_status) {
          SUCCEED();
          success = true;
          break;
        }
      }
      if (!success) {
        FAIL();
      }

      success = false;
      for (const auto &stat : datas[inst]._status) {
        if (stat == solver->SOLVER_STRING_STATUS[slv_status]) {
          SUCCEED();
          success = true;
          break;
        }
      }
      if (!success) {
        FAIL();
      }

      //========================================================================================
      // Check LP value after solve
      if (solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL") {
        double lp_val = solver->get_lp_value();
        REQUIRE(Approx(lp_val) == datas[inst]._optval);
      }

      solver->free();
      REQUIRE(solver->get_number_of_instances() == 1);
    }
  }
}

TEST_CASE("A LP problem is solved and we can get the LP solution",
          "[solve-lp][get-lp-sol]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(LP_TOY, SLACKS, REDUCED);
  SECTION("Loop on the instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance, false);

      //========================================================================================
      // Solve as LP and get solution
      int slv_status = solver->solve_lp();

      bool success = false;
      for (auto stat : datas[inst]._status_int) {
        if (stat == slv_status) {
          SUCCEED();
          success = true;
          break;
        }
      }
      if (!success) {
        FAIL();
      }

      success = false;
      for (const auto &stat : datas[inst]._status) {
        if (stat == solver->SOLVER_STRING_STATUS[slv_status]) {
          SUCCEED();
          success = true;
          break;
        }
      }
      if (!success) {
        FAIL();
      }
      if (solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL") {
        double lp_val = solver->get_lp_value();
        REQUIRE(Approx(lp_val) == datas[inst]._optval);

        std::vector<double> primals(solver->get_ncols());
        std::vector<double> duals(solver->get_nrows());
        std::vector<double> reduced(solver->get_ncols());
        solver->get_lp_sol(primals.data(), duals.data(), reduced.data());
        double solve_val;
        double expected_val;

        /* Testing primals*/
        for (int i(0); i < solver->get_ncols(); i++) {
          solve_val = primals[i];
          expected_val = datas[inst]._primals[i];
          REQUIRE(Approx(solve_val) == expected_val);
        }

        /* Testing duals*/
        for (int i(0); i < solver->get_nrows(); i++) {
          solve_val = duals[i];
          expected_val = datas[inst]._duals[i];
          REQUIRE(Approx(solve_val) == expected_val);
        }

        /* Testing reduced costs*/
        for (int i(0); i < solver->get_ncols(); i++) {
          solve_val = reduced[i];
          expected_val = datas[inst]._reduced_costs[i];
          REQUIRE(Approx(solve_val) == expected_val);
        }
      }

      solver->free();
      REQUIRE(solver->get_number_of_instances() == 1);
    }
  }
}

TEST_CASE("A problem is solved and we can get the optimal solution",
          "[solve-mip]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, LP_TOY, UNBD_PRB, INFEAS_PRB);
  SECTION("Loop on the instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      // As CLP is a pure LP solver, it cannot pass this test
      if (solver_name != "CLP") {
        std::filesystem::path instance = datas[inst]._path;
        //========================================================================================
        // Solver declaration
        SolverAbstract::Ptr solver = factory.create_solver(solver_name);
        solver->read_prob_mps(instance, false);

        //========================================================================================
        // Solve as MIP
        int slv_status = solver->solve_mip();

        bool success = false;
        for (auto stat : datas[inst]._status_int) {
          if (stat == slv_status) {
            SUCCEED();
            success = true;
            break;
          }
        }
        if (!success) {
          FAIL();
        }

        success = false;
        for (auto stat : datas[inst]._status) {
          if (stat == solver->SOLVER_STRING_STATUS[slv_status]) {
            SUCCEED();
            success = true;
            break;
          }
        }
        if (!success) {
          FAIL();
        }

        if (solver->SOLVER_STRING_STATUS[slv_status] == "OPTIMAL") {
          double mip_val = solver->get_mip_value();
          REQUIRE(mip_val == datas[inst]._optval);

          std::vector<double> primals(solver->get_ncols());
          solver->get_mip_sol(primals.data());
          double solve_val;
          double expected_val;

          /* Testing primals*/
          for (int i(0); i < solver->get_ncols(); i++) {
            solve_val = primals[i];
            expected_val = datas[inst]._primals[i];
            REQUIRE(Approx(solve_val) == expected_val);
          }
        }

        solver->free();
        REQUIRE(solver->get_number_of_instances() == 1);
      }
    }
  }
}