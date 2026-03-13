#include "antares-xpansion/benders/benders_core/strategies/SequentialSubproblemSolver.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

SequentialSubproblemSolver::SequentialSubproblemSolver() = default;

void SequentialSubproblemSolver::solve_subproblems(BendersBase& benders)
{
}

void SequentialSubproblemSolver::broadcast_master_solution(BendersBase& benders)
{
}

void SequentialSubproblemSolver::gather_cuts_and_build(BendersBase& benders)
{
}

void SequentialSubproblemSolver::update_best_solution(BendersBase& benders)
{
}

bool SequentialSubproblemSolver::should_parallelize() const
{
    return true;
}

std::string SequentialSubproblemSolver::name() const
{
    return "Sequential Subproblem Solver";
}
