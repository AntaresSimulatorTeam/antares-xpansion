#ifndef ANTARESXPANSION_ITERATIONRESULTLOG_H
#define ANTARESXPANSION_ITERATIONRESULTLOG_H

#include "core/ILogger.h"

#include "list"
#include <map>
#include <string>
namespace xpansion {
namespace logger {

typedef std::map<std::string, std::string> value_map;
typedef std::map<std::string, int> size_map;

class IterationResultLog {

public:
  std::string Log_at_iteration_end(const LogData &data);

private:
  const std::string indent_0 = "\t\t";
  const std::string indent_1 = "\t";

  const std::string LABEL = "LABEL";
  const std::string VALUE = "VALUE";
  const std::string UNIT = "UNIT";
  std::list<value_map> _values;
  size_map _max_sizes;

private:
  std::string create_solution_str(const value_map &value,
                                  const size_map &sizes);

  value_map create_value_map(const std::string &label, const std::string &value,
                             const std::string &unit);

  void setValuesFromData(const LogData &data);

  void setMaximumStringSizes();

  std::string getCompleteMessageString();
};

} // namespace logger
} // namespace xpansion

#endif // ANTARESXPANSION_ITERATIONRESULTLOG_H
