#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/strategies/IBatchStrategy.h"

class EpsilonBatchStrategy: public IBatchStrategy
{
public:
    EpsilonBatchStrategy() = default;
    ~EpsilonBatchStrategy() override = default;

    void Run(BendersCore& benders) override;

    void InitializeProblems(BendersCore& benders) override;

    [[nodiscard]] std::string name() const override;
};
