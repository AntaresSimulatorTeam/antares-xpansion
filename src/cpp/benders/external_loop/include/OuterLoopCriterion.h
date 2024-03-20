#pragma once
#include <memory>
#include <regex>

#include "BendersBase.h"
#include "LogUtils.h"
#include "common.h"

class CriterionCouldNotBeSatisfied
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

enum class CRITERION { LOW, IS_MET, HIGH };
class IOuterLoopCriterion {
 public:
  virtual CRITERION IsCriterionSatisfied(double criterion_value) = 0;
  virtual std::string StateAsString() const = 0;
  virtual double CriterionValue() const = 0;
};

class OuterloopCriterionLossOfLoad : public IOuterLoopCriterion {
 public:
  explicit OuterloopCriterionLossOfLoad(const ExternalLoopOptions& options);
  CRITERION IsCriterionSatisfied(double sum_loss) override;
  std::string StateAsString() const override;
  double CriterionValue() const override { return sum_loss_; }

 private:
  ExternalLoopOptions options_;
  double sum_loss_ = 0.0;
};
