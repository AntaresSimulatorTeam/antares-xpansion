#pragma once

#include <iostream>
#include <json/json.h>
#include <multisolver_interface/SolverAbstract.h>

struct SensitivityInputData
{
    double epsilon;
    double best_ub;
    std::map<std::string, int> name_to_id;
    SolverAbstract::Ptr last_master;
    bool capex;
    std::vector<std::string> projection;
};

class SensitivityInputReader
{
public:
    SensitivityInputReader() = default;
    SensitivityInputReader(const std::string &json_input_path);
    ~SensitivityInputReader() = default;

    SensitivityInputData get_input_data();

private:
    Json::Value _json_data;
    Json::Value _benders_data;
    std::string _last_master_path;
    std::string _structure_file_path;

    Json::Value read_json(const std::string &json_file_path);

    SolverAbstract::Ptr get_last_master();
    double get_best_ub();
    std::map<std::string, int> get_name_to_id();
    std::vector<std::string> get_projection();
};
