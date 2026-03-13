#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/strategies/ILoopStrategy.h"

class SingleLoopStrategy: public ILoopStrategy
{
public:
    SingleLoopStrategy();
    ~SingleLoopStrategy() override = default;

    void run(BendersBase& benders) override;

    [[nodiscard]] bool should_stop(const CurrentIterationData& data) const override;

    [[nodiscard]] std::string name() const override;
};
