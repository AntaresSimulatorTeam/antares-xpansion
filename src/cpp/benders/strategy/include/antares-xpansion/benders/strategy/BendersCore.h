#pragma once

#include <memory>
#include <string>

#include "IBatchingStrategy.h"
#include "IBendersCore.h"
#include "IExecutionStrategy.h"
#include "IOuterLoopStrategy.h"

/**
 * @class BendersCore
 * @brief Main orchestrator that composes execution, batching, and outer-loop strategies
 *
 * This class implements the Strategy pattern by composing three independent strategies:
 * - ExecutionStrategy: Handles Sequential or MPI parallel execution
 * - BatchingStrategy: Handles batch processing or no batching
 * - OuterLoopStrategy: Handles outer loop optimization or no outer loop
 *
 * BendersCore orchestrates these strategies to provide a complete Benders implementation
 * that can be configured at runtime by injecting different strategy combinations.
 */
class BendersCore: public IBendersCore
{
public:
    /**
     * @brief Constructor with strategy injection
     * @param execution_strategy Execution strategy (Sequential or ParallelMPI), can be nullptr
     * @param batching_strategy Batching strategy (NoBatching or ByBatch), can be nullptr
     * @param outer_loop_strategy Outer loop strategy (NoOuterLoop or OuterLoop), can be nullptr
     *
     * Note: Strategies can be nullptr. The class handles null strategies gracefully.
     * For features not needed, you can pass nullptr or use "No-op" strategies
     * (NoBatchingStrategy, NoOuterLoopStrategy) for clarity.
     */
    BendersCore(std::unique_ptr<IExecutionStrategy> execution_strategy,
                std::unique_ptr<IBatchingStrategy> batching_strategy,
                std::unique_ptr<IOuterLoopStrategy> outer_loop_strategy):
        execution_(std::move(execution_strategy)),
        batching_(std::move(batching_strategy)),
        outer_loop_(std::move(outer_loop_strategy))
    {
    }

    /**
     * @brief Orchestrates the complete Benders execution
     *
     * Coordinates the three strategies:
     * 1. Outer loop initialization (if applicable)
     * 2. Batching initialization (if applicable)
     * 3. Execution strategy initialization
     * 4. Main execution (outer loop drives execution, or execution runs directly)
     * 5. Batching updates (if applicable)
     */
    void launch() override
    {
        // Initialize outer loop data
        if (outer_loop_)
        {
            outer_loop_->init_data();
        }

        // Initialize batching
        if (batching_)
        {
            batching_->InitializeProblems();
        }

        // Initialize execution
        if (execution_)
        {
            execution_->InitializeProblems();
        }

        // Run the algorithm
        // If outer loop is active, it drives the execution
        // Otherwise, execution strategy runs directly
        if (outer_loop_)
        {
            outer_loop_->Run();
        }
        else if (execution_)
        {
            execution_->Run();
        }

        // Update batching criterion if applicable
        if (batching_)
        {
            batching_->UpdateStoppingCriterion();
        }
    }

    /**
     * @brief Set input mapping
     *
     * Delegates to the execution strategy's underlying implementation.
     */
    void set_input_map(const CouplingMap& coupling_map) override
    {
        if (execution_)
        {
            execution_->set_input_map(coupling_map);
        }
    }

    /**
     * @brief Get master row index by name
     *
     * Delegates to execution strategy.
     */
    [[nodiscard]] int MasterRowIndex(const std::string& row_name) const override
    {
        return execution_ ? execution_->MasterRowIndex(row_name) : -1;
    }

    /**
     * @brief Change master RHS value
     *
     * Delegates to execution strategy.
     */
    void MasterChangeRhs(int id_row, double val) const override
    {
        if (execution_)
        {
            execution_->MasterChangeRhs(id_row, val);
        }
    }

    /**
     * @brief Get best iteration data
     *
     * Delegates to execution strategy.
     */
    [[nodiscard]] LogData GetBestIterationData() const override
    {
        return execution_ ? execution_->GetBestIterationData() : LogData{};
    }

    /**
     * @brief Get all cuts from workers
     *
     * Delegates to execution strategy.
     */
    [[nodiscard]] WorkerMasterDataVect AllCuts() const override
    {
        return execution_ ? execution_->AllCuts() : WorkerMasterDataVect{};
    }

    /**
     * @brief Save outer loop solution to output file
     *
     * Delegates to outer loop strategy if available.
     * Note: This functionality may need to be added to IOuterLoopStrategy
     * if outer loop implementations need to expose this method.
     */
    void SaveOuterLoopSolutionInOutputFile() const override
    {
        // This is an outer loop concern, but the current interface doesn't expose it
        // For now, this is a no-op. Can be extended if IOuterLoopStrategy is enhanced.
    }

    /**
     * @brief Free allocated resources
     *
     * Delegates to execution strategy.
     */
    void free() override
    {
        if (execution_)
        {
            execution_->free();
        }
    }

    /**
     * @brief Set whether to free problems
     *
     * Delegates to execution strategy.
     */
    void DoFreeProblems(bool v) override
    {
        if (execution_)
        {
            execution_->DoFreeProblems(v);
        }
    }

    /**
     * @brief Initialize problems - delegates to all strategies
     */
    void InitializeProblems() override
    {
        if (batching_)
        {
            batching_->InitializeProblems();
        }

        if (execution_)
        {
            execution_->InitializeProblems();
        }
    }

    /**
     * @brief Get Benders algorithm name
     *
     * Returns a composite name indicating the strategy configuration.
     */
    [[nodiscard]] std::string BendersName() const override
    {
        std::string name = "BendersCore(";

        if (execution_)
        {
            name += execution_->BendersName();
        }
        else
        {
            name += "NoExecution";
        }

        name += ")";
        return name;
    }

    /**
     * @brief Get execution time
     *
     * Returns the execution time from the execution strategy.
     */
    [[nodiscard]] double execution_time() const override
    {
        return execution_ ? execution_->execution_time() : 0.0;
    }

private:
    std::unique_ptr<IExecutionStrategy> execution_;
    std::unique_ptr<IBatchingStrategy> batching_;
    std::unique_ptr<IOuterLoopStrategy> outer_loop_;
};
