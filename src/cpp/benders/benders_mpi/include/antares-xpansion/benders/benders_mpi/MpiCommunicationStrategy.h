#pragma once

#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"
#include "common_mpi.h"

/**
 * @brief Communication strategy for MPI-based distributed execution.
 *
 * This strategy wraps a boost::mpi::communicator to provide distributed
 * communication for running Benders decomposition across multiple MPI ranks.
 * Subproblem solving is distributed across ranks rather than using local TBB
 * parallelism.
 */
class MpiCommunicationStrategy: public ICommunicationStrategy
{
public:
    explicit MpiCommunicationStrategy(mpi::communicator& world):
        world_(world)
    {
    }

    [[nodiscard]] int Rank() const override
    {
        return world_.rank();
    }

    [[nodiscard]] int WorldSize() const override
    {
        return world_.size();
    }

    void Barrier() const override
    {
        world_.barrier();
    }

    [[nodiscard]] bool ShouldParallelize() const override
    {
        return false;
    }

    void Broadcast(bool& value) const override
    {
        mpi::broadcast(world_, value, 0);
    }

    void Broadcast(Point& value) const override
    {
        mpi::broadcast(world_, value, 0);
    }

    void Gather(const SubProblemDataMap& local,
                std::vector<SubProblemDataMap>& gathered) const override
    {
        mpi::gather(world_, local, gathered, 0);
    }

    void Reduce(double local, double& global) const override
    {
        mpi::reduce(world_, local, global, std::plus<double>(), 0);
    }

    void Reduce(const std::vector<double>& local, std::vector<double>& global) const override
    {
        mpi::reduce(world_, local, global, std::plus<double>(), 0);
    }

    [[nodiscard]] int AllReduceBitwiseAnd(int local) const override
    {
        int result = 0;
        mpi::all_reduce(world_, local, result, mpi::bitwise_and<int>());
        return result;
    }

    void Gather(const SubProblemNamesInCut& local,
                std::vector<SubProblemNamesInCut>& gathered) const override
    {
        mpi::gather(world_, local, gathered, 0);
    }

    void Broadcast(std::vector<std::vector<int>>& value) const override
    {
        mpi::broadcast(world_, value, 0);
    }

    [[nodiscard]] std::string Name() const override
    {
        return "Benders mpi";
    }

private:
    mpi::communicator& world_;
};
