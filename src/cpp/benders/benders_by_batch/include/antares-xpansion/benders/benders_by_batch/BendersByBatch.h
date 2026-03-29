#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#include "BatchCollection.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_mpi/MpiCommunicationStrategy.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"

class BendersByBatch: public BendersBase
{
    std::vector<unsigned> random_batch_permutation_;

public:
    BendersByBatch(const BendersBaseOptions& options,
                   std::shared_ptr<ILogger> logger,
                   std::shared_ptr<Output::OutputWriter> writer,
                   mpi::communicator& world,
                   std::shared_ptr<MathLoggerDriver> mathLoggerDriver);
    ~BendersByBatch() override = default;
    void Run() override;
    void BuildCut(const std::vector<std::string>& batch_sub_problems,
                  double* batch_contribution_in_gap,
                  std::vector<double>& external_loop_criterion_current_batch,
                  int& local_solved);

    std::string BendersName() const override
    {
        return "Benders By Batch mpi";
    }

    const int rank_0 = 0;

protected:
    void InitializeProblems() override;
    void BroadcastSingleSubpbCostsUnderApprox();
    void ComputeXCut() override;
    void UpdateStoppingCriterion() override;
    bool ShouldRelaxationStop() const override;
    void BuildBatches();

    mpi::communicator& _world;

    /// Generic broadcast for types not covered by ICommunicationStrategy
    /// (used for BatchCollection, arrays, scalars, etc.)
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

private:
    void GetSubproblemCut(SubProblemDataMap& subproblem_cut_package,
                          const std::vector<std::string>& batch_sub_problems);
    void BuildMasterProblem() override;
    double ComputeBatchContributionInGap(
      const std::vector<SubProblemDataMap>& gathered_subproblem_map,
      const std::vector<SubProblemNamesInCut>& subproblems_per_cut) const;
    void GetSubproblemCutCache(SubProblemDataMap& subproblem_data_map,
                               const std::vector<std::string>& batch_sub_problems);
    Timer calculate_subproblem_contribution(const std::string& name,
                                            PlainData::SubProblemData& subproblem_data);
    void GetSubproblemCutFast(SubProblemDataMap& subproblem_data_map,
                              const std::vector<std::string>& batch_sub_problems);

    void get_subs_per_cut_per_batch();

    BatchCollection batch_collection_;
    BatchCollection batch_collection_full_for_cuts_;
    void MasterLoop();
    void SolveBatches();
    void SeparationLoop();
    void UpdateRemainingEpsilon();
    void BroadcastXOut();
    double Gap() const;
    size_t number_of_batch_;
    unsigned current_batch_id_;
    double remaining_epsilon_;
    double cumulative_subproblems_timer_per_iter_;
    bool misprice_;
    int first_unsolved_batch_;
    int batch_counter_;
};

#endif // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
