#include "catch2.hpp"
#include "define_datas.hpp"
#include "multisolver_interface/Solver.h"

TEST_CASE("Write and read basis", "[basis]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY);
  std::string instance = datas[inst]._path;
  SECTION("Loop on solvers") for (auto const& solver_name : factory.get_solvers_list()) {
    // As CLP is a pure LP solver, it cannot pass this test
    if (solver_name != "CLP") {
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->init();
      solver->read_prob_mps(instance);
      solver->solve_mip();

      std::vector<int> rstatus(solver->get_nrows());
      std::vector<int> cstatus(solver->get_ncols());
      solver->get_basis(rstatus.data(), cstatus.data());

      std::string basis_file = std::tmpnam(nullptr);
      solver->write_basis(basis_file);


      SolverAbstract::Ptr solver_2 = factory.create_solver(solver_name);
      solver_2->init();
      solver_2->read_prob_mps(instance);
      solver_2->read_basis(basis_file);

      std::vector<int> rstatus_2(solver_2->get_nrows());
      std::vector<int> cstatus_2(solver_2->get_ncols());
      solver_2->get_basis(rstatus_2.data(), cstatus_2.data());

      for (int i(0); i < solver->get_nrows(); i++) {
        REQUIRE(rstatus[i] == rstatus_2[i]);
      }
      for (int i(0); i < solver->get_ncols(); i++) {
        REQUIRE(cstatus[i] == cstatus_2[i]);
      }

      solver->free();
      solver_2->free();
    }
  }
}