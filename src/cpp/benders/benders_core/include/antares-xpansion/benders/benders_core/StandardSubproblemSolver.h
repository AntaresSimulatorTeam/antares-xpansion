#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/ISubproblemSolver.h"

class StandardSubproblemSolver: public ISubproblemSolver
{
public:
    explicit StandardSubproblemSolver(std::shared_ptr<BendersBase> benders_base);
    void Solve(CurrentIterationData& data) override;

private:
    std::shared_ptr<BendersBase> benders_base_;
};
