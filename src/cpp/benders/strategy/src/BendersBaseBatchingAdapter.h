#pragma once

#include "antares-xpansion/benders/strategy/IBatchingStrategy.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"

class BendersBaseBatchingAdapter : public IBatchingStrategy
{
public:
    explicit BendersBaseBatchingAdapter(pBendersBase benders)
        : benders_(std::move(benders))
    {
    }

    void InitializeProblems() override { benders_->InitializeProblems(); }
    void UpdateStoppingCriterion() override { benders_->UpdateStoppingCriterion(); }
    bool ShouldRelaxationStop() const override { return benders_->ShouldRelaxationStop(); }

private:
    pBendersBase benders_;
};

