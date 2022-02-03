#include "SensitivityWriter.h"
#include "config.h"
#include <fstream>
#include <iostream>

SensitivityWriter::SensitivityWriter(const std::string &json_filename) : _filename(json_filename) {}

void SensitivityWriter::dump()
{
    _output[ANTARES_C][VERSION_C] = ANTARES_VERSION_TAG;
    _output[ANTARES_XPANSION_C][VERSION_C] = PROJECT_VER;

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
    _output[EPSILON_C] = output_data.epsilon;
    _output[BEST_BENDERS_C] = output_data.best_benders_cost;

    Json::Value pbs_data_l(Json::arrayValue);
    
    for (const auto &single_pb_data : output_data.pbs_data)
    {
        Json::Value single_pb_data_l;
        single_pb_data_l[PB_TYPE_C] = single_pb_data.pb_type;
        single_pb_data_l[OPT_DIR_C] = single_pb_data.opt_dir;
        single_pb_data_l[STATUS_C] = single_pb_data.status;
        single_pb_data_l[OBJECTIVE_C] = single_pb_data.objective;
        single_pb_data_l[SYSTEM_COST_C] = single_pb_data.system_cost;
        
        Json::Value candidates_l(Json::arrayValue);
        for (const auto &candidate : single_pb_data.candidates)
        {
            Json::Value candidate_l;
            candidate_l[NAME_C] = candidate.first;
            candidate_l[INVEST_C] = candidate.second;
            candidates_l.append(candidate_l);
        }

        single_pb_data_l[CANDIDATES_C] = candidates_l;
        pbs_data_l.append(single_pb_data_l);
    }
    _output[SENSITIVITY_SOLUTION_C] = pbs_data_l;
}

void SensitivityWriter::end_writing(SensitivityOutputData const &output_data)
{
    write_sensitivity_output(output_data);
    dump();
}