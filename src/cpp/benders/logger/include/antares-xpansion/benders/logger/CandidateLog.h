#ifndef ANTARESXPANSION_CANDIDATELOG_H
#define ANTARESXPANSION_CANDIDATELOG_H
#include <list>
#include <map>
#include <string>

#include "Commons.h"
#include "ILogger.h"

using xpansion::logger::commons::indent_0;
using xpansion::logger::commons::indent_1;
namespace xpansion {
namespace logger {
typedef std::map<std::string, std::string> value_map;
typedef std::map<std::string, int> size_map;

class CandidateLog {
 public:
  CandidateLog() = default;
  explicit CandidateLog(const std::string &line_prefix);
  std::string log_iteration_candidates(const LogData &_data);

 private:
  const std::string CANDIDATE = "CANDIDATE";
  const std::string INVEST = "INVEST";
  const std::string INVEST_MIN = "INVEST_MIN";
  const std::string INVEST_MAX = "INVEST_MAX";
  std::string _line_prefix = "";

  std::list<value_map> _values;
  size_map _sizes;

  std::string getHeaderString() const;
  std::string getMainBodyString(const LogData &data);
  void set_values_and_sizes(const LogData &data);
  static std::string get_formatted_string_from_value(double val);
  void updateMaximumSizes(value_map &valuesMap);
  std::string getStringBodyUsingValuesAndSizes();
  std::string create_candidate_str(const value_map &value);
};

}  // namespace logger
}  // namespace xpansion

#endif  // ANTARESXPANSION_CANDIDATELOG_H
