#include "OuterLoopCriterion.h"

#include "LoggerUtils.h"

OuterloopCriterionLossOfLoad::OuterloopCriterionLossOfLoad(
    const ExternalLoopOptions& options)
    : options_(options) {}

CRITERION OuterloopCriterionLossOfLoad::IsCriterionSatisfied(double sum_loss) {
  sum_loss_ = sum_loss;

  if (sum_loss_ <= options_.EXT_LOOP_CRITERION_VALUE +
                       options_.EXT_LOOP_CRITERION_TOLERANCE) {
    if (sum_loss_ >= options_.EXT_LOOP_CRITERION_VALUE -
                         options_.EXT_LOOP_CRITERION_TOLERANCE) {
      return CRITERION::IS_MET;
    }
    return CRITERION::LOW;
  } else {
    return CRITERION::HIGH;
  }
}

std::string OuterloopCriterionLossOfLoad::StateAsString() const {
  std::ostringstream msg;
  msg << "Sum loss = " << sum_loss_ << "\n"
      << "threshold: " << options_.EXT_LOOP_CRITERION_VALUE << "\n"
      << "epsilon: " << options_.EXT_LOOP_CRITERION_TOLERANCE << "\n";

  return msg.str();
}
