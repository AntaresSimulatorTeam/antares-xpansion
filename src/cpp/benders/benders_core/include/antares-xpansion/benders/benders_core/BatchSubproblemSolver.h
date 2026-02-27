#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/ISubproblemSolver.h"

class BatchSubproblemSolver: public ISubproblemSolver
{
public:
    explicit BatchSubproblemSolver(std::shared_ptr<BendersBase> benders_base):
        benders_base_(std::move(benders_base))
    {
    }

    void Solve(CurrentIterationData& data) override;

private:
    std::shared_ptr<BendersBase> benders_base_;
    void SeparationLoop();
};
