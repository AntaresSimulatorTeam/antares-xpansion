#include <algorithm>
#include <iomanip>
#include <list>
#include <map>
#include <sstream>

#include "Commons.h"
#include "IterationResultLog.h"
#include "core/ILogger.h"

namespace xpansion {
namespace logger {

std::string IterationResultLog::Log_at_iteration_end(const LogData &data) {
  setValuesFromData(data);
  setMaximumStringSizes();
  return getCompleteMessageString();
}

void IterationResultLog::setValuesFromData(const LogData &data) {

  double low_bd = data.lb;
  double abs_gap = data.best_ub - data.lb;
  double rel_gap = abs_gap / data.best_ub;
  const double overall_cost = data.slave_cost + data.invest_cost;

  // Quick and dirty fix when gap is negative, further investigation needed
  if (abs_gap < 0) {
    abs_gap = 0;
    rel_gap = 0;
    low_bd = data.best_ub;
  }

  // Get values

  _values.clear();
  _values.push_back(create_value_map(
      "Operational cost", commons::create_str_million_euros(data.slave_cost),
      " Me"));
  _values.push_back(create_value_map(
      "Investment cost", commons::create_str_million_euros(data.invest_cost),
      " Me"));
  _values.push_back(create_value_map(
      "Overall cost", commons::create_str_million_euros(overall_cost), " Me"));
  _values.push_back(create_value_map(
      "Best Solution", commons::create_str_million_euros(data.best_ub), " Me"));
  _values.push_back(create_value_map(
      "Lower Bound", commons::create_str_million_euros(low_bd), " Me"));
  _values.push_back(create_value_map("Absolute gap",
                                     commons::create_str_euros(abs_gap), " e"));
  _values.push_back(
      create_value_map("Relative gap", commons::create_str_euros(rel_gap), ""));
}

void IterationResultLog::setMaximumStringSizes() { // Compute maximum string
                                                   // size

  for (auto value : _values) {
    _max_sizes[LABEL] =
        std::max<int>(value.at(LABEL).length(), _max_sizes[LABEL]);
    _max_sizes[VALUE] =
        std::max<int>(value.at(VALUE).length(), _max_sizes[VALUE]);
  }
}

std::string IterationResultLog::getCompleteMessageString() {
  std::stringstream _stream;
  _stream << indent_0 << "Solution =" << std::endl;
  for (const auto &value : _values) {
    _stream << create_solution_str(value, _max_sizes) << std::endl;
  }
  return _stream.str();
}

inline std::string
IterationResultLog::create_solution_str(const value_map &value,
                                        const size_map &sizes) {
  std::stringstream result;
  result << indent_0 << indent_1 << std::setw(sizes.at(LABEL))
         << value.at(LABEL);
  result << " = " << std::setw(sizes.at(VALUE)) << value.at(VALUE)
         << value.at(UNIT);
  return result.str();
}

inline value_map IterationResultLog::create_value_map(const std::string &label,
                                                      const std::string &value,
                                                      const std::string &unit) {
  value_map result;
  result[LABEL] = label;
  result[VALUE] = value;
  result[UNIT] = unit;

  return result;
}

} // namespace logger
} // namespace xpansion