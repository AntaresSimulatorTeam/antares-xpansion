#include "antares-xpansion/benders/benders_core/strategies/BatchSubproblemSolver.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

BatchSubproblemSolver::BatchSubproblemSolver() = default;

void BatchSubproblemSolver::solve_subproblems(BendersBase& benders)
{
}

void BatchSubproblemSolver::broadcast_master_solution(BendersBase& benders)
{
}

void BatchSubproblemSolver::gather_cuts_and_build(BendersBase& benders)
{
}

void BatchSubproblemSolver::update_best_solution(BendersBase& benders)
{
}

bool BatchSubproblemSolver::should_parallelize() const
{
    return false;
}

std::string BatchSubproblemSolver::name() const
{
    return "Batch Subproblem Solver";
}
