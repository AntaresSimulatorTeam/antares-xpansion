#include <fstream>
#include "common.h"
#include "OutputWriter.h"
#include "SensitivityInputReader.h"

SensitivityInputReader::SensitivityInputReader(const std::string &last_master_mps_path, const std::string &benders_out_json_path, const std::string &structure_file_path, double epsilon) : _last_master_path(last_master_mps_path), _structure_file_path(structure_file_path), _epsilon(epsilon)
{
    _json_data = read_json(benders_out_json_path);
}

SolverAbstract::Ptr SensitivityInputReader::read_last_master()
{
    SolverFactory factory;
    SolverAbstract::Ptr last_master;

    if (_json_data[Output::OPTIONS_C]["SOLVER_NAME"].asString() == "COIN")
    {
        last_master = factory.create_solver("CBC");
    }
    else
    {
        last_master = factory.create_solver(_json_data[Output::OPTIONS_C]["SOLVER_NAME"].asString());
    }
    last_master->init();
    last_master->set_threads(1);
    last_master->set_output_log_level(_json_data[Output::OPTIONS_C]["LOG_LEVEL"].asInt());
    last_master->read_prob_mps(_last_master_path);
    return last_master;
}

Json::Value SensitivityInputReader::read_json(const std::string &json_file_path)
{
    std::ifstream json_file(json_file_path);
    Json::Value json_data;
    if (json_file.good())
    {
        json_file >> json_data;
    }
    else
    {
        throw std::runtime_error("unable to open : " + json_file_path);
    }
    return json_data;
}

double SensitivityInputReader::read_best_ub()
{
    return _json_data[Output::SOLUTION_C][Output::OVERALL_COST_C].asDouble();
}

std::map<int, std::string> SensitivityInputReader::get_id_to_name()
{
    std::map<int, std::string> id_to_name;
    std::ifstream structure(_structure_file_path);
    if (structure.good())
    {
        std::string line;

        while (std::getline(structure, line))
        {
            std::stringstream buffer(line);
            std::string problem_name;
            std::string variable_name;
            int variable_id;
            buffer >> problem_name;
            buffer >> variable_name;
            buffer >> variable_id;

            if (problem_name == _json_data[Output::OPTIONS_C]["MASTER_NAME"].asString())
            {
                id_to_name[variable_id] = variable_name;
            }
        }
    }
    else
    {
        throw std::runtime_error("unable to open : " + _structure_file_path);
    }
    return id_to_name;
}