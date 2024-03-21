#include "OuterLoopCriterion.h"

#include <numeric>

#include "LoggerUtils.h"

OuterloopCriterionLossOfLoad::OuterloopCriterionLossOfLoad(
    const ExternalLoopOptions& options)
    : options_(options) {}

CRITERION OuterloopCriterionLossOfLoad::IsCriterionSatisfied(
    const std::vector<double>& criterion_value) {
  criterion_values_ = criterion_value;
  for (const auto& criterion_value : criterion_values_) {
    // options_.EXT_LOOP_CRITERION_VALUE --> to  vect
    // options_.EXT_LOOP_CRITERION_TOLERANCE --> to  vect
    if (criterion_value <= options_.EXT_LOOP_CRITERION_VALUE +
                               options_.EXT_LOOP_CRITERION_TOLERANCE) {
      if (criterion_value >= options_.EXT_LOOP_CRITERION_VALUE -
                                 options_.EXT_LOOP_CRITERION_TOLERANCE) {
        return CRITERION::IS_MET;
      }
      return CRITERION::LOW;
    } else {
      return CRITERION::HIGH;
    }
  }
}

double OuterloopCriterionLossOfLoad::SumCriterions() const {
  return std::accumulate(criterion_values_.begin(), criterion_values_.end(),
                         0.0);
}
std::string OuterloopCriterionLossOfLoad::StateAsString() const {
  std::ostringstream msg;
  auto sum_loss =
      std::accumulate(criterion_values_.begin(), criterion_values_.end(), 0.0);
  msg << "Sum loss = " << sum_loss << "\n"
      << "threshold: " << options_.EXT_LOOP_CRITERION_VALUE << "\n"
      << "epsilon: " << options_.EXT_LOOP_CRITERION_TOLERANCE << "\n";

  return msg.str();
}
