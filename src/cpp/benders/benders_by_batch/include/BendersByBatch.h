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
  std::string BendersName() const { return "Benders By Batch mpi"; }

 protected:
  void InitializeProblems() override;
  void BroadcastSingleSubpbCostsUnderApprox();

 private:
  void GetSubproblemCut(SubProblemDataMap &subproblem_cut_package,
                        const std::vector<std::string> &batch_sub_problems,
                        double *sum) const;
  BatchCollection batch_collection_;
};

#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
