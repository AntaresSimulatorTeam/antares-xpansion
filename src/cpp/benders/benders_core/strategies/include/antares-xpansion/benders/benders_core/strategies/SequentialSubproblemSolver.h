#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/strategies/ISubproblemSolver.h"

class SequentialSubproblemSolver: public ISubproblemSolver
{
public:
    SequentialSubproblemSolver();
    ~SequentialSubproblemSolver() override = default;

    void solve_subproblems(BendersBase& benders) override;

    void broadcast_master_solution(BendersBase& benders) override;

    void gather_cuts_and_build(BendersBase& benders) override;

    void update_best_solution(BendersBase& benders) override;

    [[nodiscard]] bool should_parallelize() const override;

    [[nodiscard]] std::string name() const override;
};
