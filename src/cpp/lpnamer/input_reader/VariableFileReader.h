//

#ifndef ANTARESXPANSION_VARIABLEFILEREADER_H
#define ANTARESXPANSION_VARIABLEFILEREADER_H

#include <ActiveLinks.h>
#include <ColumnToChange.h>

#include <string>
#include <vector>

struct VariableFileReadNameConfiguration {
  std::string ntc_variable_name;
  std::string cost_origin_variable_name;
  std::string cost_extremite_variable_name;
};

class VariableFileReader {
 public:
  VariableFileReader(
      const std::string& fileName, const std::vector<ActiveLink>& links,
      const VariableFileReadNameConfiguration& variable_name_config,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger);

  const std::vector<std::string>& getVariables() const;
  const std::map<linkId, ColumnsToChange>& getNtcVarColumns() const;
  const std::map<linkId, ColumnsToChange>& getDirectCostVarColumns() const;
  const std::map<linkId, ColumnsToChange>& getIndirectCostVarColumns() const;

 private:
  std::string getVarNameFromLine(const std::string& line) const;

  std::vector<std::string> _variables;
  std::map<linkId, ColumnsToChange> _ntc_p_var_columns;
  std::map<linkId, ColumnsToChange> _indirect_cost_p_var_columns;
  std::map<linkId, ColumnsToChange> _direct_cost_p_var_columns;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  ProblemGenerationLog::ProblemGenerationLogger& loggerRef_ = *logger_;
};

#endif  // ANTARESXPANSION_VARIABLEFILEREADER_H
