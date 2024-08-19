#pragma once
#include "BendersMPI.h"
#include "CriterionComputation.h"
namespace Outerloop {

class BendersMpiOuterLoop : public BendersMpi {
 public:
  ~BendersMpiOuterLoop() override = default;
  BendersMpiOuterLoop(BendersBaseOptions const &options, Logger logger,
                      Writer writer, mpi::environment &env,
                      mpi::communicator &world,
                      std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                      CriterionComputation &criterion_computation);

 protected:
  void GatherCuts(const SubProblemDataMap &subproblem_data_map,
                  const Timer &walltime) override;

 public:
  void launch() override;

 protected:
  void InitializeProblems() override;

  virtual void ComputeSubproblemsContributionToOuterLoopCriterion(
      const SubProblemDataMap &subproblem_data_map);
  void SolveSubproblem(
      SubProblemDataMap &subproblem_data_map,
      PlainData::SubProblemData &subproblem_data, const std::string &name,
      const std::shared_ptr<SubproblemWorker> &worker) override;
  void SetSubproblemsVariablesIndex();
  void UpdateOuterLoopMaxCriterionArea();

 private:
  CriterionComputation &criterion_computation_;
};

}  // namespace Outerloop
