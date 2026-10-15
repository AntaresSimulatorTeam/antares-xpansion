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

    void BroadcastBool(bool& value) const override
    {
        mpi::broadcast(world_, value, 0);
    }

    template<typename T>
    void BroadCast(T& value, int root) const
    {
        mpi::broadcast(world_, value, root);
    }

    template<typename T>
    void BroadCast(T* values, int n, int root) const
    {
        mpi::broadcast(world_, values, n, root);
    }

    template<typename T, typename Op>
    void Reduce(const T& in_value, T& out_value, Op op, int root) const
    {
        mpi::reduce(world_, in_value, out_value, op, root);
    }

    template<typename T, typename Op>
    void AllReduce(const T& in_value, T& out_value, Op op) const
    {
        mpi::all_reduce(world_, in_value, out_value, op);
    }

    /// Access the underlying MPI communicator for operations not yet
    /// abstracted into the strategy interface.
    mpi::communicator& World()
    {
        return world_;
    }

    const mpi::communicator& World() const
    {
        return world_;
    }

private:
    mpi::communicator& world_;
};
