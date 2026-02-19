#pragma once

#include <memory>

#include "IExecutionStrategy.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"

/**
 * @class SequentialExecutionStrategy
 * @brief Execution strategy adapter for BendersSequential
 *
 * Wraps a BendersBase-derived implementation to implement IExecutionStrategy.
 */
class SequentialExecutionStrategy: public IExecutionStrategy
{
public:
    // Accept any BendersBase-derived implementation (BendersSequential, etc.)
    explicit SequentialExecutionStrategy(std::unique_ptr<BendersBase> sequential):
        sequential_(std::move(sequential))
    {
    }

    void launch() override
    {
        if (sequential_)
        {
            sequential_->launch();
        }
    }

    void InitializeProblems() override
    {
        if (sequential_)
        {
            sequential_->InitializeProblems();
        }
    }

    void Run() override
    {
        if (sequential_)
        {
            // Run is protected in derived classes; launch() orchestrates run lifecycle.
            sequential_->launch();
        }
    }

    [[nodiscard]] std::string BendersName() const override
    {
        return sequential_ ? sequential_->BendersName()
                           : std::string("SequentialExecutionStrategy");
    }

    [[nodiscard]] double execution_time() const override
    {
        return sequential_ ? sequential_->execution_time() : 0.0;
    }

    void set_input_map(const CouplingMap& coupling_map) override
    {
        if (sequential_)
        {
            sequential_->set_input_map(coupling_map);
        }
    }

    [[nodiscard]] int MasterRowIndex(const std::string& row_name) const override
    {
        return sequential_ ? sequential_->MasterRowIndex(row_name) : -1;
    }

    void MasterChangeRhs(int id_row, double val) const override
    {
        if (sequential_)
        {
            sequential_->MasterChangeRhs(id_row, val);
        }
    }

    [[nodiscard]] LogData GetBestIterationData() const override
    {
        return sequential_ ? sequential_->GetBestIterationData() : LogData{};
    }

    [[nodiscard]] WorkerMasterDataVect AllCuts() const override
    {
        return sequential_ ? sequential_->AllCuts() : WorkerMasterDataVect{};
    }

    void free() override
    {
        if (sequential_)
        {
            // Call through base type: BendersBase::free() is a public virtual method.
            sequential_->free();
        }
    }

    void DoFreeProblems(bool v) override
    {
        if (sequential_)
        {
            sequential_->DoFreeProblems(v);
        }
    }

private:
    std::unique_ptr<BendersBase> sequential_;
};
