#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#include "BatchCollection.h"
#include "BendersMPI.h"
#include "common_mpi.h"

class BendersByBatch : public BendersMpi {
  std::vector<unsigned> random_batch_permutation_;

 public:
  BendersByBatch(BendersBaseOptions const &options, Logger logger,
                 Writer writer, mpi::environment &env,
                 mpi::communicator &world);
  ~BendersByBatch() override = default;
  void Run() override;
  void BuildCut(const std::vector<std::string> &batch_sub_problems,
                double *sum);
  std::string BendersName() const override { return "Benders By Batch mpi"; }

 protected:
  void InitializeProblems() override;
  void BroadcastSingleSubpbCostsUnderApprox();

 private:
  void GetSubproblemCut(
      SubProblemDataMap &subproblem_cut_package,
      const std::vector<std::string> &batch_sub_problems,
      double *batch_subproblems_costs_contribution_in_gap_per_proc);
  BatchCollection batch_collection_;
  void MasterLoop();
  int SolveBatches();
  int SeparationLoop();
  void UpdateRemainingEpsilon();
  void BroadcastXOut();
  size_t number_of_batch_;
  unsigned current_batch_id_;
  int number_of_sub_problem_resolved_;
  double remaining_epsilon_;
  double cumulative_subproblems_timer_per_iter_;
  bool misprice_;
};

#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
