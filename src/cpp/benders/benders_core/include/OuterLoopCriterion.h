#pragma once
#include <memory>
#include <regex>

#include "BendersBase.h"

enum class CRITERION { LESSER, EQUAL, GREATER };
class IOuterLoopCriterion {
 public:
  virtual CRITERION IsCriterionSatisfied(const BendersCuts& benders_cuts) = 0;
};

class OuterloopCriterionLOL : public IOuterLoopCriterion {
 public:
  explicit OuterloopCriterionLOL(double threshold, double epsilon);
  CRITERION IsCriterionSatisfied(const BendersCuts& benders_cuts) override;

 private:
  double ProcessSum(const BendersCuts& benders_cuts);
  const std::string positive_unsupplied_vars_prefix_ =
      "^PositiveUnsuppliedEnergy::";
  const std::regex rgx_ = std::regex(positive_unsupplied_vars_prefix_);
  const int UNSUPPLIED_ENERGY_MAX = 1;
  double threshold_ = 1e6;
  double epsilon_ = 1e-4;
};
