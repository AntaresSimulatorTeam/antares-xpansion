#pragma once

#include <memory>
#include <string>

#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/common.h"

class BendersBase;

class ISubproblemSolver
{
public:
    virtual ~ISubproblemSolver() = default;

    virtual void solve_subproblems(BendersBase& benders) = 0;

    virtual void broadcast_master_solution(BendersBase& benders) = 0;

    virtual void gather_cuts_and_build(BendersBase& benders) = 0;

    virtual void update_best_solution(BendersBase& benders) = 0;

    [[nodiscard]] virtual bool should_parallelize() const = 0;

    [[nodiscard]] virtual std::string name() const = 0;
};

using SubproblemSolverPtr = std::unique_ptr<ISubproblemSolver>;
