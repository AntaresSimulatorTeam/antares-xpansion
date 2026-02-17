#pragma once

#include "IBendersCore.h"
#include "IExecutionStrategy.h"
#include "IBatchingStrategy.h"
#include "IOuterLoopStrategy.h"
#include <memory>
#include <string>

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
class BendersCore : public IBendersCore
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
                std::unique_ptr<IOuterLoopStrategy> outer_loop_strategy)
        : execution_(std::move(execution_strategy)),
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
     * Note: This is typically handled by the execution strategy's underlying
     * implementation. For now, this is a placeholder that would need to be
     * properly integrated with the execution strategy.
     */
    void set_input_map(const CouplingMap& coupling_map) override
    {
        // TODO: This needs to be delegated to the underlying implementation
        // For now, this is a placeholder as the current strategy interfaces
        // don't expose this method directly
    }

    /**
     * @brief Get master row index by name
     * 
     * Note: Delegated to execution strategy's underlying implementation.
     * Returns -1 if not available.
     */
    [[nodiscard]] int MasterRowIndex(const std::string& row_name) const override
    {
        // TODO: This needs to be delegated to the underlying implementation
        // For now, return a safe default
        return -1;
    }

    /**
     * @brief Change master RHS value
     * 
     * Note: Delegated to execution strategy's underlying implementation.
     */
    void MasterChangeRhs(int id_row, double val) const override
    {
        // TODO: This needs to be delegated to the underlying implementation
    }

    /**
     * @brief Get best iteration data
     * 
     * Note: Delegated to execution strategy's underlying implementation.
     */
    [[nodiscard]] LogData GetBestIterationData() const override
    {
        // TODO: This needs to be delegated to the underlying implementation
        return LogData{};
    }

    /**
     * @brief Get all cuts from workers
     * 
     * Note: Delegated to execution strategy's underlying implementation.
     */
    [[nodiscard]] WorkerMasterDataVect AllCuts() const override
    {
        // TODO: This needs to be delegated to the underlying implementation
        return WorkerMasterDataVect{};
    }

    /**
     * @brief Save outer loop solution to output file
     * 
     * Delegates to outer loop strategy if available.
     */
    void SaveOuterLoopSolutionInOutputFile() const override
    {
        // This is an outer loop concern, but the current interface doesn't expose it
        // TODO: Extend IOuterLoopStrategy if needed
    }

    /**
     * @brief Free allocated resources
     * 
     * Note: Delegated to execution strategy's underlying implementation.
     */
    void free() override
    {
        // TODO: This needs to be delegated to the underlying implementation
    }

    /**
     * @brief Set whether to free problems
     * 
     * Note: Delegated to execution strategy's underlying implementation.
     */
    void DoFreeProblems(bool v) override
    {
        // TODO: This needs to be delegated to the underlying implementation
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
