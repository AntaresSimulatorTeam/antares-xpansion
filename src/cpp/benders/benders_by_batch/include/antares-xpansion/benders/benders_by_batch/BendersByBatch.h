#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#include "BatchCollection.h"
#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"

/*!
 * \class BendersByBatch
 * \brief Class for batch-based benders decomposition
 * \deprecated Use BendersCore instead - will be removed in future version
 */
class [[deprecated("Use BendersCore instead")]] BendersByBatch: public BendersMpi
{
    std::vector<unsigned> random_batch_permutation_;

public:
    using BendersMpi::BendersMpi;
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

protected:
    void InitializeProblems() override;
    void BroadcastSingleSubpbCostsUnderApprox();
    void ComputeXCut() override;
    void UpdateStoppingCriterion() override;
    bool ShouldRelaxationStop() const override;

private:
    void GetSubproblemCut(SubProblemDataMap& subproblem_cut_package,
                          const std::vector<std::string>& batch_sub_problems);
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
