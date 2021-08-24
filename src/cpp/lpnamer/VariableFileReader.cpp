//

#include <sstream>

#include "VariableFileReader.h"

VariableFileReader::VariableFileReader(const std::string& fileName, const std::vector<ActiveLink>& links,
                                       const std::string& ntc_variable_name) {

    std::string line;
    std::ifstream file(fileName.c_str());
    if (!file.good()) {
        throw std::runtime_error("Unable to open '" + fileName +"'");
    }
    while (std::getline(file, line)) {

        std::string name = getVarNameFromLine(line);
        _variables.push_back(name);

        std::istringstream buffer(line);
        colId id;
        std::string variable;
        buffer >> id;
        buffer >> variable;

        if (variable == ntc_variable_name) {
            int pays;
            int link_id;
            int time_step;
            buffer >> pays;
            buffer >> link_id;
            buffer >> time_step;

            auto it = std::find_if(links.begin(), links.end(), [link_id] (const ActiveLink& link){
                return link._idLink == link_id;
            });

            if (it != links.end()){
                _ntc_p_var_columns[link_id].push_back({id, time_step});
            }
        }
    }
    file.close();

}

std::string VariableFileReader::getVarNameFromLine(const std::string &line) const {
    std::ostringstream name;
    {
        std::istringstream buffer(line);
        std::string part;
        bool is_first(true);
        while (buffer >> part) {
            if (!is_first) {
                name << part << "_";
            }
            else {
                is_first = false;
            }
        }
    }
    return name.str();
}

std::vector<std::string> VariableFileReader::getVariables() const {

    return _variables;
}

std::map<linkId,  ColumnsToChange> VariableFileReader::getNtcVarColumns() const{
    return _ntc_p_var_columns;
}