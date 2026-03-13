#include "antares-xpansion/benders/benders_core/strategies/MPISubproblemSolver.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

MPISubproblemSolver::MPISubproblemSolver() = default;

void MPISubproblemSolver::solve_subproblems(BendersBase& benders)
{
    benders.GetSubproblemCut(benders.subproblem_map);
}

void MPISubproblemSolver::broadcast_master_solution(BendersBase& benders)
{
    Point x_cut = benders.get_x_cut();
    benders.set_x_cut(x_cut);
}

void MPISubproblemSolver::gather_cuts_and_build(BendersBase& benders)
{
}

void MPISubproblemSolver::update_best_solution(BendersBase& benders)
{
    benders.compute_ub();
    benders.update_best_ub();
}

bool MPISubproblemSolver::should_parallelize() const
{
    return false;
}

std::string MPISubproblemSolver::name() const
{
    return "MPI Subproblem Solver";
}
