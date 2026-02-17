#pragma once

#include "IExecutionStrategy.h"
#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include <memory>

/**
 * @class ParallelMpiExecutionStrategy
 * @brief Execution strategy adapter for BendersMPI
 * 
 * Wraps BendersMPI to implement IExecutionStrategy interface.
 * This adapter allows the MPI-based parallel Benders implementation to be used
 * within the Strategy pattern composition.
 * 
 * The MPI communicator is managed externally and passed by reference to BendersMPI.
 */
class ParallelMpiExecutionStrategy : public IExecutionStrategy
{
public:
    /**
     * @brief Constructor that takes ownership of a BendersMPI instance
     * @param mpi_benders Unique pointer to BendersMPI implementation
     * 
     * Note: The MPI communicator must be provided to BendersMPI at construction
     * and must remain valid for the lifetime of this strategy.
     */
    explicit ParallelMpiExecutionStrategy(std::unique_ptr<BendersMpi> mpi_benders)
        : mpi_benders_(std::move(mpi_benders))
    {
    }

    void launch() override
    {
        if (mpi_benders_)
        {
            mpi_benders_->launch();
        }
    }

    void InitializeProblems() override
    {
        if (mpi_benders_)
        {
            mpi_benders_->InitializeProblems();
        }
    }

    void Run() override
    {
        // BendersMPI's Run() is protected, but launch() calls it internally
        // Delegate to launch() which handles the full MPI execution
        if (mpi_benders_)
        {
            mpi_benders_->launch();
        }
    }

    [[nodiscard]] std::string BendersName() const override
    {
        return mpi_benders_ ? mpi_benders_->BendersName() : "ParallelMpiExecutionStrategy";
    }

    [[nodiscard]] double execution_time() const override
    {
        return mpi_benders_ ? mpi_benders_->execution_time() : 0.0;
    }

    // Master problem interaction
    void set_input_map(const CouplingMap& coupling_map) override
    {
        if (mpi_benders_)
        {
            mpi_benders_->set_input_map(coupling_map);
        }
    }

    [[nodiscard]] int MasterRowIndex(const std::string& row_name) const override
    {
        return mpi_benders_ ? mpi_benders_->MasterRowIndex(row_name) : -1;
    }

    void MasterChangeRhs(int id_row, double val) const override
    {
        if (mpi_benders_)
        {
            mpi_benders_->MasterChangeRhs(id_row, val);
        }
    }

    // Results and data access
    [[nodiscard]] LogData GetBestIterationData() const override
    {
        return mpi_benders_ ? mpi_benders_->GetBestIterationData() : LogData{};
    }

    [[nodiscard]] WorkerMasterDataVect AllCuts() const override
    {
        return mpi_benders_ ? mpi_benders_->AllCuts() : WorkerMasterDataVect{};
    }

    // Resource management
    void free() override
    {
        if (mpi_benders_)
        {
            mpi_benders_->free();
        }
    }

    void DoFreeProblems(bool v) override
    {
        if (mpi_benders_)
        {
            mpi_benders_->DoFreeProblems(v);
        }
    }

private:
    std::unique_ptr<BendersMpi> mpi_benders_;
};

