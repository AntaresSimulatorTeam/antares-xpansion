#pragma once

#include <memory>
#include <string>

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/strategies/ILoopStrategy.h"
#include "antares-xpansion/benders/benders_core/strategies/ISubproblemSolver.h"

class BendersCore: public BendersBase
{
public:
    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                SubproblemSolverPtr solver_strategy,
                LoopStrategyPtr loop_strategy);

    void launch() override;
    std::string BendersName() const override;
    void InitializeProblems() override;
    void free() override;
    [[nodiscard]] bool shouldParallelize() const override;

protected:
    void Run() override;
    void post_run_actions() const override;

private:
    SubproblemSolverPtr solver_strategy_;
    LoopStrategyPtr loop_strategy_;
};
