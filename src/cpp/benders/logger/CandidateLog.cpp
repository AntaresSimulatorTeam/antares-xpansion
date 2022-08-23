#include "CandidateLog.h"

#include <algorithm>
#include <iomanip>
#include <list>
#include <map>
#include <sstream>

#include "CandidateLog.h"
#include "core/ILogger.h"

namespace xpansion {
namespace logger {

CandidateLog::CandidateLog(const std::string &line_prefix)
    : _line_prefix(line_prefix) {}

std::string CandidateLog::log_iteration_candidates(const LogData &data) {
  std::stringstream _stream;
  _stream << getHeaderString();
  _stream << getMainBodyString(data);
  return _stream.str();
}

std::string CandidateLog::getHeaderString() const {
  std::stringstream header;
  header << _line_prefix << indent_0 << "Candidates:" << std::endl;
  return header.str();
}

std::string CandidateLog::getMainBodyString(const LogData &data) {
  _values.clear();
  _sizes.clear();
  set_values_and_sizes(data);
  return getStringBodyUsingValuesAndSizes();
}

void CandidateLog::set_values_and_sizes(const LogData &data) {
  for (const auto &pairVarnameValue : data.x_out) {
    std::string candidate = pairVarnameValue.first;

    value_map valuesMap;
    valuesMap[CANDIDATE] = candidate;
    valuesMap[INVEST] =
        get_formatted_string_from_value(pairVarnameValue.second);
    valuesMap[INVEST_MIN] =
        get_formatted_string_from_value(data.min_invest.at(candidate));
    valuesMap[INVEST_MAX] =
        get_formatted_string_from_value(data.max_invest.at(candidate));
    _values.push_back(valuesMap);

    updateMaximumSizes(valuesMap);
  }
}

inline std::string CandidateLog::get_formatted_string_from_value(double val) {
  std::stringstream result;
  result << std::fixed << std::setprecision(2) << val;
  return result.str();
}

void CandidateLog::updateMaximumSizes(
    value_map &valuesMap) {  // Compute maximum string size
  for (const auto &it : valuesMap) {
    const std::string &key = it.first;
    _sizes[key] = std::max<int>(it.second.length(), _sizes[key]);
  }
}

std::string CandidateLog::getStringBodyUsingValuesAndSizes() {
  std::stringstream main_body;
  for (const auto &value : _values) {
    main_body << create_candidate_str(value) << std::endl;
  }
  return main_body.str();
}

inline std::string CandidateLog::create_candidate_str(const value_map &value) {
  std::stringstream result;
  result << _line_prefix << indent_0 << indent_1
         << std::setw(_sizes.at(CANDIDATE)) << value.at(CANDIDATE);
  result << " = " << std::setw(_sizes.at(INVEST)) << value.at(INVEST)
         << " invested MW ";
  result << "-- possible interval [" << std::setw(_sizes.at(INVEST_MIN))
         << value.at(INVEST_MIN);
  result << "; " << std::setw(_sizes.at(INVEST_MAX)) << value.at(INVEST_MAX)
         << "] MW";
  return result.str();
}

}  // namespace logger
}  // namespace xpansion