//

#ifndef ANTARESXPANSION_VARIABLEFILEREADER_H
#define ANTARESXPANSION_VARIABLEFILEREADER_H

#include <string>
#include <vector>

#include <ActiveLinks.h>
#include <ProblemModifier.h>

class VariableFileReader {

public:

    VariableFileReader(const std::string& fileName, const std::vector<ActiveLink>& links,
                       const std::string& ntc_variable_name);

    const std::vector<std::string>& getVariables() const;
    const std::map<linkId,  ColumnsToChange>& getNtcVarColumns() const;


private:

    std::string getVarNameFromLine(const std::string &line) const;

    std::vector<std::string> _variables;
    std::map<linkId,  ColumnsToChange> _ntc_p_var_columns;
};


#endif //ANTARESXPANSION_VARIABLEFILEREADER_H
