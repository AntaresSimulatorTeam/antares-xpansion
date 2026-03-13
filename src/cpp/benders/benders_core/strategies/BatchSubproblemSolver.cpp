#include "antares-xpansion/benders/benders_core/strategies/BatchSubproblemSolver.h"

#include "antares-xpansion/benders/benders_core/BendersBase.h"

BatchSubproblemSolver::BatchSubproblemSolver() = default;

void BatchSubproblemSolver::solve_subproblems(BendersBase& benders)
{
    benders.GetSubproblemCut(benders.subproblem_map);
}

void BatchSubproblemSolver::broadcast_master_solution(BendersBase& benders)
{
    Point x_cut = benders.get_x_cut();
    benders.set_x_cut(x_cut);
}

void BatchSubproblemSolver::gather_cuts_and_build(BendersBase& benders)
{
}

void BatchSubproblemSolver::update_best_solution(BendersBase& benders)
{
    benders.compute_ub();
    benders.update_best_ub();
}

bool BatchSubproblemSolver::should_parallelize() const
{
    return false;
}

std::string BatchSubproblemSolver::name() const
{
    return "Batch Subproblem Solver";
}
