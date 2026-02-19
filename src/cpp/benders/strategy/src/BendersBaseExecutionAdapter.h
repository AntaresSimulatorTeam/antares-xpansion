#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/strategy/IExecutionStrategy.h"

class BendersBaseExecutionAdapter: public IExecutionStrategy
{
public:
    explicit BendersBaseExecutionAdapter(pBendersBase benders):
        benders_(std::move(benders))
    {
    }

    void launch() override
    {
        benders_->launch();
    }

    void InitializeProblems() override
    {
        benders_->InitializeProblems();
    }

    void Run() override
    { /* forward to BendersBase::Run via protected? use launch as entry */
    }

    std::string BendersName() const override
    {
        return benders_->BendersName();
    }

    double execution_time() const override
    {
        return benders_->execution_time();
    }

private:
    pBendersBase benders_;
};
