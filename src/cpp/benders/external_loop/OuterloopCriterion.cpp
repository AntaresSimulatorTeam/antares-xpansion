#include "OuterLoopCriterion.h"

#include <algorithm>
#include <numeric>

#include "LoggerUtils.h"
bool OuterloopCriterionLossOfLoad::DoubleCompare(double a, double b) {
  return a > b + options_.EXT_LOOP_CRITERION_TOLERANCE;
}
OuterloopCriterionLossOfLoad::OuterloopCriterionLossOfLoad(
    const ExternalLoopOptions& options)
    : options_(options) {}

bool OuterloopCriterionLossOfLoad::IsCriterionHigh(
    const std::vector<double>& criterion_values) {
  // tmp EXT_LOOP_CRITERION_VALUES must be a vector of size
  // criterion_values.size()
  EXT_LOOP_CRITERION_VALUES_ = std::vector<double>(
      criterion_values.size(), options_.EXT_LOOP_CRITERION_VALUE);
  // si une zone est depassé sur au moins
  criterion_values_ = criterion_values;
  // options_.EXT_LOOP_CRITERION_VALUE --> to  vect
  // options_.EXT_LOOP_CRITERION_TOLERANCE --> to  vect

  // return std::equal(criterion_value.begin(), criterion_value.end(),
  //                   options_.EXT_LOOP_CRITERION_VALUE.begin(),
  //                   DoubleCompare);
  // return std::equal(criterion_values.begin(), criterion_values.end(),
  //                   EXT_LOOP_CRITERION_VALUES_.begin(),
  //                   &OuterloopCriterionLossOfLoad::DoubleCompare);

  for (int index(0); index < criterion_values_.size(); ++index) {
    if (criterion_values_[index] > EXT_LOOP_CRITERION_VALUES_[index] +
                                       options_.EXT_LOOP_CRITERION_TOLERANCE) {
      return true;
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
