#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/strategies/ILoopStrategy.h"

class OuterLoopStrategy: public ILoopStrategy
{
public:
    explicit OuterLoopStrategy();
    ~OuterLoopStrategy() override = default;

    void run(BendersBase& benders) override;

    [[nodiscard]] bool should_stop(const CurrentIterationData& data) const override;

    [[nodiscard]] std::string name() const override;
};
