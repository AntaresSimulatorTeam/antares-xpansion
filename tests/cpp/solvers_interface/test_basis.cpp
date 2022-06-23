#include "catch2.hpp"
#include "define_datas.hpp"
#include "multisolver_interface/Solver.h"

TEST_CASE("Writing and reading basis", "[basis]") {
  std::cout << "Writing and reading base test" << std::endl;
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY);
  std::string instance = datas[inst]._path;
  std::string solver_name = "CBC";
  SolverAbstract::Ptr solver = factory.create_solver(solver_name);
  solver->init();
  solver->read_prob_mps(instance);

  std::string basis_file =
      "/home/bittartho/development/antares-xpansion/test_basis.bss";

  solver->solve_mip();

  std::vector<int> rstatus(solver->get_nrows());
  std::vector<int> cstatus(solver->get_ncols());
  solver->get_basis(rstatus.data(), cstatus.data());

  solver->write_basis(basis_file);
  SolverAbstract::Ptr solver_2 = factory.create_solver(solver_name);
  solver_2->init();
  solver_2->read_prob_mps(instance);

  solver_2->read_basis(basis_file);

  std::vector<int> rstatus_2(solver_2->get_nrows());
  std::vector<int> cstatus_2(solver_2->get_ncols());
  solver_2->get_basis(rstatus_2.data(), cstatus_2.data());

  std::cout << "Row status" << std::endl;
  for (int i(0); i < solver->get_nrows(); i++) {
    // REQUIRE(rstatus[i] == rstatus_2[i]);
    std::cout << rstatus[i] << " " << rstatus_2[i] << std::endl;
  }
  std::cout << "Col status" << std::endl;
  for (int i(0); i < solver->get_ncols(); i++) {
    // REQUIRE(cstatus[i] == cstatus_2[i]);
    std::cout << cstatus[i] << " " << cstatus_2[i] << std::endl;
  }
  solver->free();
  solver_2->free();
}