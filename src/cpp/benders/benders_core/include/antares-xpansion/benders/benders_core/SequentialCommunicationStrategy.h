#pragma once

#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"

/**
 * @brief Communication strategy for single-process (sequential) execution.
 *
 * This strategy is used when running Benders decomposition on a single process.
 * All communication operations are local no-ops, and subproblem solving uses
 * TBB for local thread parallelism.
 */
class SequentialCommunicationStrategy: public ICommunicationStrategy
{
public:
    [[nodiscard]] int Rank() const override
    {
        return 0;
    }

    [[nodiscard]] int WorldSize() const override
    {
        return 1;
    }

    void Barrier() const override
    {
        // No-op for single process
    }

    [[nodiscard]] bool ShouldParallelize() const override
    {
        return true;
    }

    void Broadcast(bool& /*value*/) const override
    {
        // No-op: single process already has the value
    }

    void Broadcast(Point& /*value*/) const override
    {
        // No-op: single process already has the value
    }

    void Gather(const SubProblemDataMap& local,
                std::vector<SubProblemDataMap>& gathered) const override
    {
        gathered = {local};
    }

    void Reduce(double local, double& global) const override
    {
        global = local;
    }

    void Reduce(const std::vector<double>& local, std::vector<double>& global) const override
    {
        global = local;
    }

    [[nodiscard]] int AllReduceBitwiseAnd(int local) const override
    {
        return local;
    }

    void Gather(const SubProblemNamesInCut& local,
                std::vector<SubProblemNamesInCut>& gathered) const override
    {
        gathered = {local};
    }

    void Broadcast(std::vector<std::vector<int>>& /*value*/) const override
    {
        // No-op: single process already has the value
    }

    [[nodiscard]] std::string Name() const override
    {
        return "Sequential";
    }
};
