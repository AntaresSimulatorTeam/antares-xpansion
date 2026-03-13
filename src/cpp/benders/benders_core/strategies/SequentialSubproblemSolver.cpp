#include "antares-xpansion/benders/benders_core/strategies/SequentialSubproblemSolver.h"

#include <execution>

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"

SequentialSubproblemSolver::SequentialSubproblemSolver() = default;

void SequentialSubproblemSolver::solve_subproblems(BendersBase& benders)
{
    SubProblemDataMap subproblem_data_map;
    benders.GetSubproblemCut(subproblem_data_map);
}

void SequentialSubproblemSolver::broadcast_master_solution(BendersBase& benders)
{
}

void SequentialSubproblemSolver::gather_cuts_and_build(BendersBase& benders)
{
}

void SequentialSubproblemSolver::update_best_solution(BendersBase& benders)
{
    benders.compute_ub();
    benders.update_best_ub();
}

bool SequentialSubproblemSolver::should_parallelize() const
{
    return true;
}

std::string SequentialSubproblemSolver::name() const
{
    return "Sequential Subproblem Solver";
}
