#include <catch2/catch_all.hpp>

#include "antares-xpansion/multisolver_interface/Solver.h"
#include "define_datas.hpp"

void assert_basis_equality(std::shared_ptr<SolverAbstract> expec_solver,
                           std::shared_ptr<SolverAbstract> current_solver)
{
    std::vector<int> expec_rstatus(expec_solver->get_nrows());
    std::vector<int> expec_cstatus(expec_solver->get_ncols());
    expec_solver->get_basis(expec_rstatus.data(), expec_cstatus.data());

    std::vector<int> current_rstatus(current_solver->get_nrows());
    std::vector<int> current_cstatus(current_solver->get_ncols());
    current_solver->get_basis(current_rstatus.data(), current_cstatus.data());

    for (int i(0); i < expec_solver->get_nrows(); i++)
    {
        REQUIRE(expec_rstatus[i] == current_rstatus[i]);
    }
    for (int i(0); i < expec_solver->get_ncols(); i++)
    {
        REQUIRE(expec_cstatus[i] == current_cstatus[i]);
    }
}

TEST_CASE("Write and read basis", "[basis]")
{
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, LP_TOY, MULTIKP, NET_MASTER, SLACKS, REDUCED);
    auto instance = datas[inst]._path;

    SECTION("Loop on instances and solvers")
    {
        for (const auto& solver_name: factory.get_solvers_list())
        {
            // As CLP is a pure LP solver, it cannot pass this test
            if (solver_name != "CLP")
            {
                std::shared_ptr<SolverAbstract> expec_solver = factory.create_solver(solver_name);
                expec_solver->read_prob_mps(instance);
                expec_solver->solve_mip();

                std::filesystem::path basis_file = std::tmpnam(nullptr);
                expec_solver->write_basis(basis_file);

                std::shared_ptr<SolverAbstract> current_solver = factory.create_solver(solver_name);
                current_solver->read_prob_mps(instance);
                current_solver->read_basis(basis_file);

                assert_basis_equality(expec_solver, current_solver);

                expec_solver->free();
                current_solver->free();
            }
        }
    }
}
