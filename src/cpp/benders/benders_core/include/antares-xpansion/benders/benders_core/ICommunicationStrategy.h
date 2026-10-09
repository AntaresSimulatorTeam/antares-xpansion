#pragma once

/**
 * @brief Strategy interface for communication operations in Benders decomposition.
 *
 * This interface abstracts the communication layer (MPI vs sequential) from the
 * Benders algorithm. It follows the Strategy Pattern to decouple the parallelism
 * dimension from the algorithmic logic:
 *
 * - SequentialCommunicationStrategy: single-process, uses TBB for local parallelism
 * - MpiCommunicationStrategy: multi-process, distributed via MPI
 *
 * By injecting this strategy into BendersBase, the same algorithmic code can run
 * in different execution contexts without inheritance-based specialization.
 *
 * Concrete subclasses choose their strategy internally and do not expose the
 * injection in their own constructors:
 *
 *   // BendersMpi wraps an mpi::communicator and builds MpiCommunicationStrategy internally:
 *   auto benders = std::make_unique<BendersMpi>(options, logger, writer, world, mathLoggerDriver);
 *
 *   // BendersSequential builds SequentialCommunicationStrategy internally (no world needed):
 *   auto benders = std::make_unique<BendersSequential>(options, logger, writer, mathLoggerDriver);
 *
 * Direct strategy injection is available through the BendersBase constructor for
 * custom subclasses (e.g., test doubles or future execution backends):
 *
 *   auto strategy = std::make_shared<MyCustomStrategy>();
 *   // Pass as the optional last argument to BendersBase:
 *   //   BendersBase(options, logger, writer, mathLoggerDriver, strategy)
 */
class ICommunicationStrategy
{
public:
    virtual ~ICommunicationStrategy() = default;

    /// Return the rank of this process (0 for sequential)
    [[nodiscard]] virtual int Rank() const = 0;

    /// Return the total number of processes (1 for sequential)
    [[nodiscard]] virtual int WorldSize() const = 0;

    /// Whether this process is the master (rank 0)
    [[nodiscard]] bool IsMaster() const
    {
        return Rank() == 0;
    }

    /// Synchronization barrier across all processes (no-op for sequential)
    virtual void Barrier() const = 0;

    /// Whether subproblems should be parallelized locally using TBB.
    /// Returns true for sequential (local parallelism), false for MPI
    /// (distribution handles parallelism across ranks).
    [[nodiscard]] virtual bool ShouldParallelize() const = 0;

    /// Broadcast a boolean value from the master process (rank 0) to all
    /// other processes. No-op for sequential (single process).
    virtual void BroadcastBool(bool& value) const = 0;
};
