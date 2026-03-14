#pragma once

#include <memory>
#include <string>

#include <boost/mpi.hpp>

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/strategies/IBatchStrategy.h"
#include "antares-xpansion/benders/benders_core/strategies/ILoopStrategy.h"
#include "antares-xpansion/benders/benders_core/strategies/ISubproblemSolver.h"

namespace mpi = boost::mpi;

class BendersCore: public BendersBase
{
public:
    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                mpi::communicator* world);

    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                SubproblemSolverPtr solver_strategy,
                LoopStrategyPtr loop_strategy);

    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                SubproblemSolverPtr solver_strategy,
                LoopStrategyPtr loop_strategy,
                BatchStrategyPtr batch_strategy);

    BendersCore(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                mpi::communicator* world,
                SubproblemSolverPtr solver_strategy,
                LoopStrategyPtr loop_strategy,
                BatchStrategyPtr batch_strategy);

    void launch() override;
    std::string BendersName() const override;
    void InitializeProblems() override;
    void free() override;
    [[nodiscard]] bool shouldParallelize() const override;

    void RunCore();

    int Rank() const;
    int WorldSize() const;

    template<typename T>
    void BroadCast(T& value, int root) const
    {
        if (world_ && *world_)
        {
            mpi::broadcast(*world_, value, root);
        }
    }

    template<typename T>
    void BroadCast(T* values, int n, int root) const
    {
        if (world_ && *world_)
        {
            mpi::broadcast(*world_, values, n, root);
        }
    }

    template<typename T>
    void Gather(const T& value, std::vector<T>& vector_of_values, int root) const
    {
        if (world_ && *world_)
        {
            mpi::gather(*world_, value, vector_of_values, root);
        }
    }

    template<typename T, typename Op>
    void Reduce(const T& in_value, T& out_value, Op op, int root) const
    {
        if (world_ && *world_)
        {
            mpi::reduce(*world_, in_value, out_value, op, root);
        }
    }

    template<typename T, typename Op>
    void AllReduce(const T& in_value, T& out_value, Op op) const
    {
        if (world_ && *world_)
        {
            mpi::all_reduce(*world_, in_value, out_value, op);
        }
    }

    void Barrier() const
    {
        if (world_ && *world_)
        {
            world_->barrier();
        }
    }

protected:
    void Run() override;
    void post_run_actions() const override;
    void BuildCut();

private:
    SubproblemSolverPtr solver_strategy_;
    LoopStrategyPtr loop_strategy_;
    BatchStrategyPtr batch_strategy_;
    mpi::communicator* world_ = nullptr;
};
