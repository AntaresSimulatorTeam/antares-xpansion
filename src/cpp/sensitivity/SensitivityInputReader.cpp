#include <fstream>
#include "common.h"
#include "OutputWriter.h"
#include "SensitivityInputReader.h"

const std::string BENDERS_OUT_C("benders_output");
const std::string LAST_MASTER_C("last_master");
const std::string STRUCTURE_C("structure");
const std::string EPSILON_C("epsilon");
const std::string CAPEX_C("capex");
const std::string PROJECTION_C("projection");

SensitivityInputReader::SensitivityInputReader(const std::string &json_input_path)
{
    _json_data = read_json(json_input_path);
    _last_master_path = _json_data[LAST_MASTER_C].asString();
    _structure_file_path = _json_data[STRUCTURE_C].asString();
    _benders_data = read_json(_json_data[BENDERS_OUT_C].asString());
}

std::vector<std::string> SensitivityInputReader::get_projection()
{
    std::vector<std::string> projection;
    for (const auto &candidate_name : _json_data[PROJECTION_C])
    {
        projection.push_back(candidate_name.asString());
    }
    return projection;
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

SolverAbstract::Ptr SensitivityInputReader::get_last_master()
{
    SolverFactory factory;
    SolverAbstract::Ptr last_master;

    if (_benders_data[Output::OPTIONS_C]["SOLVER_NAME"].asString() == "COIN")
    {
        last_master = factory.create_solver("CBC");
    }
    else
    {
        last_master = factory.create_solver(_benders_data[Output::OPTIONS_C]["SOLVER_NAME"].asString());
    }
    last_master->init();
    last_master->set_threads(1);
    last_master->set_output_log_level(_benders_data[Output::OPTIONS_C]["LOG_LEVEL"].asInt());
    last_master->read_prob_mps(_last_master_path);
    return last_master;
}

double SensitivityInputReader::get_best_ub()
{
    return _benders_data[Output::SOLUTION_C][Output::OVERALL_COST_C].asDouble();
}

std::map<std::string, int> SensitivityInputReader::get_name_to_id()
{
    std::map<std::string, int> _name_to_id;
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

            if (problem_name == _benders_data[Output::OPTIONS_C]["MASTER_NAME"].asString())
            {
                _name_to_id[variable_name] = variable_id;
            }
        }
    }
    else
    {
        throw std::runtime_error("unable to open : " + _structure_file_path);
    }
    return _name_to_id;
}

SensitivityInputData SensitivityInputReader::get_input_data()
{
    SensitivityInputData input_data;
    input_data.epsilon = _json_data[EPSILON_C].asDouble();
    input_data.best_ub = get_best_ub();
    input_data.name_to_id = get_name_to_id();
    input_data.last_master = get_last_master();
    input_data.capex = _json_data[CAPEX_C].asBool();
    input_data.projection = get_projection();
    return input_data;
}