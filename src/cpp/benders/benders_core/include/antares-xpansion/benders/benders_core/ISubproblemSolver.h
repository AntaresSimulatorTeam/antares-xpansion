#pragma once

#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"

class ISubproblemSolver
{
public:
    virtual ~ISubproblemSolver() = default;

    virtual void Solve(CurrentIterationData& data) = 0;
};
