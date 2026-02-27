#include "antares-xpansion/benders/benders_core/StandardSubproblemSolver.h"

StandardSubproblemSolver::StandardSubproblemSolver(std::shared_ptr<BendersBase> benders_base):
    benders_base_(std::move(benders_base))
{
}

void StandardSubproblemSolver::Solve(CurrentIterationData& data)
{
    benders_base_->BuildCut();
}
