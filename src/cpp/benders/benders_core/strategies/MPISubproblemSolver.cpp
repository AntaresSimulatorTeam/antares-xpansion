#include "antares-xpansion/benders/benders_core/strategies/MPISubproblemSolver.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

MPISubproblemSolver::MPISubproblemSolver() = default;

void MPISubproblemSolver::solve_subproblems(BendersBase& benders)
{
}

void MPISubproblemSolver::broadcast_master_solution(BendersBase& benders)
{
}

void MPISubproblemSolver::gather_cuts_and_build(BendersBase& benders)
{
}

void MPISubproblemSolver::update_best_solution(BendersBase& benders)
{
}

bool MPISubproblemSolver::should_parallelize() const
{
    return false;
}

std::string MPISubproblemSolver::name() const
{
    return "MPI Subproblem Solver";
}
