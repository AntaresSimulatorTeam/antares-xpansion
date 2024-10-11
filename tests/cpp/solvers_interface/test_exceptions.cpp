#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "catch2.hpp"
#include "antares-xpansion/multisolver_interface/Solver.h"

TEST_CASE("InvalidStatusException", "[exceptions][invalid_status]") {
  SolverFactory factory;
  for (auto const& solver_name : factory.get_solvers_list()) {
    //========================================================================================
    // solver declaration
    SolverAbstract::Ptr solver = factory.create_solver(solver_name);
    try {
      solver->zero_status_check(-1, "test exception", "");
    } catch (InvalidStatusException& ex) {
      REQUIRE(ex.ErrorMessage() ==
              "Failed to test exception: invalid status -1 (0 expected)");
    }
  }
}

TEST_CASE("InvalidRowSizeException", "[exceptions][invalid_row_size]") {
  SolverFactory factory;
  std::string solver_name = "CBC";
  //========================================================================================
  // solver declaration
  SolverAbstract::Ptr solver = factory.create_solver(solver_name);

  try {
    std::vector<std::string> names = solver->get_row_names(0, 9);
  } catch (InvalidRowSizeException& ex) {
    REQUIRE(ex.ErrorMessage() ==
            "Invalid row size for solver. 0 rows available (10 expected)");
  }
}

TEST_CASE("InvalidColSizeException", "[exceptions][invalid_col_size]") {
  SolverFactory factory;
  std::string solver_name = "CBC";
  //========================================================================================
  // solver declaration
  SolverAbstract::Ptr solver = factory.create_solver(solver_name);
  solver->init();

  try {
    std::vector<std::string> names = solver->get_col_names(0, 9);
  } catch (InvalidColSizeException& ex) {
    REQUIRE(ex.ErrorMessage() ==
            "Invalid col size for solver. 0 cols available (10 expected)");
  }
}

SolverAbstract::Ptr createSimpleProblem(const std::string& solver_name) {
  SolverFactory factory;
  SolverAbstract::Ptr solver = factory.create_solver(solver_name);
  solver->init();

  // 2 Colunms with no obj
  std::vector<int> matind(0);
  std::vector<double> matval(0);
  std::vector<int> matstart = std::vector<int>(2, 0);
  std::vector<double> obj = std::vector<double>(2, 0.0);
  std::vector<double> lb = std::vector<double>(2, 0.0);
  std::vector<double> ub = std::vector<double>(2, 100.0);
  solver->add_cols(2, 0, obj.data(), matstart.data(), matind.data(),
                   matval.data(), lb.data(), ub.data());
  //========================================================================================
  // Add a row to problem : creating row structure
  int newRows = 2;
  int newElems = 3;
  std::vector<double> matvalRow = {5.0, -3.2, 10};
  std::vector<int> mind = {0, 1, 1};
  std::vector<int> mstart = {0, 2, 3};
  std::vector<char> ntype = {'L', 'G'};
  std::vector<double> rhs = {5.0, 2};
  //========================================================================================
  // Add rows to solver
  solver->add_rows(newRows, newElems, ntype.data(), rhs.data(), NULL,
                   mstart.data(), mind.data(), matvalRow.data());

  return solver;
}

TEST_CASE("InvalidColTypeException", "[exceptions][invalid_col_type]") {
  for (auto const& solver_name : {"CBC", "CLP"}) {
    SolverAbstract::Ptr solver = createSimpleProblem(solver_name);
    try {
      solver->chg_col_type(std::vector<int>(1, 0), std::vector<char>(1, 'P'));
    } catch (InvalidColTypeException& ex) {
      REQUIRE(ex.ErrorMessage() == "Invalid col type P for solver.");
    }
  }
}

TEST_CASE("InvalidBoundTypeException", "[exceptions][invalid_bound_type]") {
  for (auto const& solver_name : {"CBC", "CLP"}) {
    SolverAbstract::Ptr solver = createSimpleProblem(solver_name);

    try {
      solver->chg_bounds(std::vector<int>(1, 0), std::vector<char>(1, 'P'),
                         std::vector<double>(1, 0));
    } catch (InvalidBoundTypeException& ex) {
      REQUIRE(ex.ErrorMessage() == "Invalid bound type P for solver.");
    }
  }
}

TEST_CASE("InvalidSetAlgorithm", "[exceptions][invalid_set_algorithm]") {
  for (auto const& solver_name : {"CBC", "CLP"}) {
    SolverFactory factory;
    SolverAbstract::Ptr solver = factory.create_solver(solver_name);

    try {
      solver->set_algorithm("BARRIER");
    } catch (InvalidSolverOptionException& ex) {
      REQUIRE(ex.ErrorMessage() ==
              "Invalid option 'set_algorithm : BARRIER' for solver.");
    }
  }
}

TEST_CASE("InvalidSetSimpleIter", "[exceptions][set_simplex_iter]") {
  for (auto const& solver_name : {"CBC"}) {
    SolverFactory factory;
    SolverAbstract::Ptr solver = factory.create_solver(solver_name);

    try {
      solver->set_simplex_iter(10);
    } catch (InvalidSolverOptionException& ex) {
      REQUIRE(ex.ErrorMessage() ==
              "Invalid option 'set_simplex_iter : 10' for solver.");
    }
  }
}

TEST_CASE("InvalidSetOptimatilityGap", "[exceptions][set_optimality_gap]") {
  for (auto const& solver_name : {"CBC", "CLP"}) {
    SolverFactory factory;
    SolverAbstract::Ptr solver = factory.create_solver(solver_name);

    try {
      solver->set_optimality_gap(10);
    } catch (InvalidSolverOptionException& ex) {
      REQUIRE(ex.ErrorMessage() ==
              "Invalid option 'set_optimality_gap : 10.000000' for solver.");
    }
  }
}

TEST_CASE("InvalidSolverNameException", "[exceptions][invalid_solver_name]") {
  for (std::string const& solver_name : {"SIRIUS", "GUROBI"}) {
    SolverFactory factory;
    try {
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
    } catch (InvalidSolverNameException& ex) {
      REQUIRE(ex.ErrorMessage() ==
              std::string("Solver '" + solver_name + "' not supported"));
    }
  }
}
