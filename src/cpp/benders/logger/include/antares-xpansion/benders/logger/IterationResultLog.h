#ifndef ANTARESXPANSION_ITERATIONRESULTLOG_H
#define ANTARESXPANSION_ITERATIONRESULTLOG_H
#include <list>
#include <map>
#include <string>

#include "Commons.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

using xpansion::logger::commons::indent_0;
using xpansion::logger::commons::indent_1;
namespace xpansion {
namespace logger {

typedef std::map<std::string, std::string> value_map;
typedef std::map<std::string, int> size_map;

class IterationResultLog {
 public:
  IterationResultLog() = default;
  explicit IterationResultLog(const std::string &line_prefix);
  std::string Log_at_iteration_end(const LogData &data);

 private:
  const std::string LABEL = "LABEL";
  const std::string VALUE = "VALUE";
  const std::string UNIT = "UNIT";
  std::list<value_map> _values;
  size_map _max_sizes;
  std::string _line_prefix = "";

 private:
  std::string create_solution_str(const value_map &value,
                                  const size_map &sizes) const;

  value_map create_value_map(const std::string &label, const std::string &value,
                             const std::string &unit) const;

  void setValuesFromData(const LogData &data);

  void setMaximumStringSizes();

  std::string getCompleteMessageString() const;
};

}  // namespace logger
}  // namespace xpansion

#endif  // ANTARESXPANSION_ITERATIONRESULTLOG_H
