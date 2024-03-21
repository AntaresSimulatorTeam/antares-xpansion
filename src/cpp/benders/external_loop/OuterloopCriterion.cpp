#include "OuterLoopCriterion.h"

#include "LoggerUtils.h"

OuterloopCriterionLossOfLoad::OuterloopCriterionLossOfLoad(
    const ExternalLoopOptions& options)
    : options_(options) {}

CRITERION OuterloopCriterionLossOfLoad::IsCriterionSatisfied(
    const std::vector<double>& criterion_value) {
  criterion_values_ = criterion_value;
  for (criterion_value : criterion_values_) {
    // options_.EXT_LOOP_CRITERION_VALUE --> to  vect
    // options_.EXT_LOOP_CRITERION_TOLERANCE --> to  vect
    if (criterion_value <= options_.EXT_LOOP_CRITERION_VALUE +
                               options_.EXT_LOOP_CRITERION_TOLERANCE) {
      if (criterion_values >= options_.EXT_LOOP_CRITERION_VALUE -
                                  options_.EXT_LOOP_CRITERION_TOLERANCE) {
        return CRITERION::IS_MET;
      }
      return CRITERION::LOW;
    } else {
      return CRITERION::HIGH;
    }
  }
}

std::string OuterloopCriterionLossOfLoad::StateAsString() const {
  std::ostringstream msg;
  msg << "Sum loss = " << criterion_values_ << "\n"
      << "threshold: " << options_.EXT_LOOP_CRITERION_VALUE << "\n"
      << "epsilon: " << options_.EXT_LOOP_CRITERION_TOLERANCE << "\n";

  return msg.str();
}
