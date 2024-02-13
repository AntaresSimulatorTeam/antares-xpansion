#pragma once
#include <memory>
#include <regex>

#include "BendersBase.h"
#include "LogUtils.h"

class CriterionCouldNotBeSatisfied
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};
// renomer satisfait, trop_faibleetc..
enum class CRITERION { LESSER, EQUAL, GREATER };
class IOuterLoopCriterion {
 public:
  virtual CRITERION IsCriterionSatisfied(
      const WorkerMasterData& worker_master_data) = 0;
  virtual std::string StateAsString() const = 0;
};

class OuterloopCriterionLOL : public IOuterLoopCriterion {
 public:
  explicit OuterloopCriterionLOL(double threshold, double epsilon);
  CRITERION IsCriterionSatisfied(
      const WorkerMasterData& milp_solution) override;
  std::string StateAsString() const override;

 private:
  void ProcessSum(const WorkerMasterData& worker_master_data);
  const std::string positive_unsupplied_vars_prefix_ =
      "^PositiveUnsuppliedEnergy::";
  const std::regex rgx_ = std::regex(positive_unsupplied_vars_prefix_);
  const int UNSUPPLIED_ENERGY_MAX = 1;
  double threshold_ = 1e6;
  double epsilon_ = 1e-4;
  double sum_loss_ = 0.0;
};
