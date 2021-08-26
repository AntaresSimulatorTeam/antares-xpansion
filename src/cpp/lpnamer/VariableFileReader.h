//

#ifndef ANTARESXPANSION_VARIABLEFILEREADER_H
#define ANTARESXPANSION_VARIABLEFILEREADER_H

#include <string>
#include <vector>

#include <ActiveLinks.h>
#include <ProblemModifier.h>

struct VariableFileReadNameConfiguration {
    std::string ntc_variable_name;
    std::string cost_origin_variable_name;
    std::string cost_extremite_variable_name;
};

class VariableFileReader {

public:

    VariableFileReader(const std::string& fileName, const std::vector<ActiveLink>& links,
                       const VariableFileReadNameConfiguration& variable_name_config);

    const std::vector<std::string>& getVariables() const;
    const std::map<linkId,  ColumnsToChange>& getNtcVarColumns() const;
    const std::map<linkId, ColumnsToChange>& getCostVarColumns() const;

private:

    std::string getVarNameFromLine(const std::string &line) const;

    std::vector<std::string> _variables;
    std::map<linkId,  ColumnsToChange> _ntc_p_var_columns;
    std::map<linkId,  ColumnsToChange> _cost_p_var_columns;
};


#endif //ANTARESXPANSION_VARIABLEFILEREADER_H
