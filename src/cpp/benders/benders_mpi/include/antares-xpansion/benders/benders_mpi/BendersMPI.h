#pragma once

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

    void launch() override;

    std::string BendersName() const override
    {
        return "Benders mpi";
    }

    const int rank_0 = 0;

protected:
    void free() override;
    void Run() override;
    void InitializeProblems() override;
    void BroadcastXCut();
    void master_build_cuts(const std::vector<SubProblemDataMap>& gathered_subproblem_map);
    void SetSubproblemDataCostAndSimplexIter(
      const std::vector<SubProblemDataMap>& gathered_subproblem_map);

    mpi::communicator& _world;

private:
    void step_1_solve_master();
    void step_2_solve_subproblems_and_build_cuts();
    void step_4_update_best_solution(int rank);

    std::vector<SubProblemNamesInCut> get_subs_per_cut(const std::vector<SubProblemNamesInCut>&,
                                                       int);

    SubProblemDataMap get_subproblem_cut_package();

    void solve_master_and_create_trace();

    void do_solve_master_create_trace_and_update_cuts();

    virtual void gather_subproblems_cut_package_and_build_cuts(
      const SubProblemDataMap& subproblem_data_map,
      const Timer& process_timer);

    void write_exception_message(const std::exception& ex) const;

    void check_if_some_proc_had_a_failure(int success);

    std::vector<SubProblemNamesInCut> subproblem_per_cut_indices_;

protected:
    void ResetMaster();

    [[nodiscard]] bool shouldParallelize() const final
    {
        return false;
    }

    void PreRunInitialization();

    int Rank() const
    {
        return _world.rank();
    }

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

    virtual void BuildMasterProblem();

    int WorldSize() const
    {
        return _world.size();
    }

    void Barrier() const
    {
        _world.barrier();
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

    virtual void GatherCuts(const SubProblemDataMap& subproblem_data_map, const Timer& walltime);
    void BroadCastVariablesIndices();
    virtual void ComputeSubproblemsContributionToCriteria(
      const SubProblemDataMap& subproblem_data_map);
    void SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                         const std::string& name,
                         const std::shared_ptr<SubproblemWorker>& worker) override;
    void UpdateMaxCriterionArea();
};

struct Entry
{
    const std::string* name = nullptr;
    int vecPos = -1;
};
