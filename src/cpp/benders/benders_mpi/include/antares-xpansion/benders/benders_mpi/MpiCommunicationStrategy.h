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

    /// Access the underlying MPI communicator for operations not yet
    /// abstracted into the strategy interface (broadcast, gather, reduce).
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
