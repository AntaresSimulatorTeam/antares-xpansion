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
  virtual CRITERION IsCriterionSatisfied(
      const std::vector<double>& criterion_value) = 0;
  virtual std::string StateAsString() const = 0;
  virtual std::vector<double> CriterionValues() const = 0;
  virtual double SumCriterions() const = 0;
};

class OuterloopCriterionLossOfLoad : public IOuterLoopCriterion {
 public:
  explicit OuterloopCriterionLossOfLoad(const ExternalLoopOptions& options);
  CRITERION IsCriterionSatisfied(
      const std::vector<double>& criterion_values) override;
  std::string StateAsString() const override;
  std::vector<double> CriterionValues() const override {
    return criterion_values_;
  }
  double SumCriterions() const override;

 private:
  ExternalLoopOptions options_;
  std::vector<double> criterion_values_ = {};
};
