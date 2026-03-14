#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/strategies/IBatchStrategy.h"

class StandardBatchStrategy: public IBatchStrategy
{
public:
    StandardBatchStrategy() = default;
    ~StandardBatchStrategy() override = default;

    void Run(BendersCore& benders) override;

    void InitializeProblems(BendersCore& benders) override;

    [[nodiscard]] std::string name() const override;
};
