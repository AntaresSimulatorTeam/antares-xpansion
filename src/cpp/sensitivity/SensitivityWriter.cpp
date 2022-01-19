#include "SensitivityWriter.h"
#include "config.h"
#include <fstream>
#include <iostream>

SensitivityWriter::SensitivityWriter(const std::string &json_filename) : _filename(json_filename){}

void SensitivityWriter::dump()
{
    _output["antares"]["version"] = ANTARES_VERSION_TAG;
    _output["antares_xpansion"]["version"] = PROJECT_VER;

    std::ofstream jsonOut_l(_filename);
    if (jsonOut_l)
    {
        jsonOut_l << _output << std::endl;
    }
    else
    {
        std::cout << "Impossible d'ouvrir le fichier json " << _filename << std::endl;
    }
}

void SensitivityWriter::write_sensitivity_output(const SensitivityOutputData &output_data)
{
    _output["epsilon"] = output_data.epsilon;
    _output["best_overall_cost"] = output_data.best_overall_cost;
    _output["sensitivity_solution_overall_cost"] = output_data.sensitivity_solution_overall_cost;
    _output["sensitivity_pb_objective"] = output_data.sensitivity_pb_objective;

    Json::Value candidates_l(Json::arrayValue);
    for (const auto &candidate : output_data.sensitivity_candidates)
    {
        Json::Value candidate_l;
        candidate_l["name"] = candidate.name;
        candidate_l["invest"] = candidate.invest;
        candidates_l.append(candidate_l);
    }
    _output["sensitivity_solution_candidates"] = candidates_l;
}

void SensitivityWriter::end_writing(SensitivityOutputData const &output_data){
    write_sensitivity_output(output_data);
    dump();
}