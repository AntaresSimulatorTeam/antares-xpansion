#pragma once
#include <memory>
#include <regex>

#include "BendersBase.h"
#include "CutsManagement.h"
class IOuterLoopCriterion {
 public:
  virtual bool IsCriterionSatisfied(const BendersCuts& benders_cuts) = 0;
};

class OuterloopCriterionLOL : public IOuterLoopCriterion {
 public:
  explicit OuterloopCriterionLOL(double threshold, double epsilon);
  bool IsCriterionSatisfied(const BendersCuts& benders_cuts) override;

 private:
  bool ProcessSum(const BendersCuts& benders_cuts);
  const std::string positive_unsupplied_vars_prefix_ =
      "^PositiveUnsuppliedEnergy::";
  const std::regex rgx_(positive_unsupplied_vars_prefix_);
  const int UNSUPPLIED_ENERGY_MAX = 1;
  double threshold_ = 1e6;
  double epsilon_ = 1e-4;
};
