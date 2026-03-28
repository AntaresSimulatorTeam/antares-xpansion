#pragma once

#include "MpiCommunicationStrategy.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/benders_core/Worker.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common_mpi.h"

/*!
 * \class BendersMpi
 * \brief Class use run the benders algorithm in parallel
 */
class BendersMpi: public BendersBase
{
public:
    ~BendersMpi() override = default;
    BendersMpi(const BendersBaseOptions& options,
               std::shared_ptr<ILogger> logger,
               std::shared_ptr<Output::OutputWriter> writer,
               mpi::communicator& world,
               std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

    std::string BendersName() const override
    {
        return "Benders mpi";
    }

    const int rank_0 = 0;

protected:
    void free() override;
    void InitializeProblems() override;

    mpi::communicator& _world;

    /// Generic broadcast for types not covered by ICommunicationStrategy
    /// (used by BendersByBatch for BatchCollection, arrays, etc.)
    template<typename T>
    void BroadCast(T& value, int root) const
    {
        mpi::broadcast(_world, value, root);
    }

    template<typename T>
    void BroadCast(T* values, int n, int root) const
    {
        mpi::broadcast(_world, values, n, root);
    }

    template<typename T>
    void Gather(const T& value, std::vector<T>& vector_of_values, int root) const
    {
        mpi::gather(_world, value, vector_of_values, root);
    }

    template<typename T, typename Op>
    void Reduce(const T& in_value, T& out_value, Op op, int root) const
    {
        mpi::reduce(_world, in_value, out_value, op, root);
    }

    template<typename T, typename Op>
    void AllReduce(const T& in_value, T& out_value, Op op) const
    {
        mpi::all_reduce(_world, in_value, out_value, op);
    }

    int Rank() const
    {
        return _world.rank();
    }

    int WorldSize() const
    {
        return _world.size();
    }

    void Barrier() const
    {
        _world.barrier();
    }

    void BroadCastVariablesIndices();
    void InitializeMaster();
    virtual void BuildMasterProblem();
    void SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                         const std::string& name,
                         const std::shared_ptr<SubproblemWorker>& worker) override;
};
