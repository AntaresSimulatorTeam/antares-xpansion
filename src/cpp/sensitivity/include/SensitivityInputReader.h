#pragma once

#include <iostream>
#include <json/json.h>
#include <multisolver_interface/SolverAbstract.h>

class SensitivityInputReader
{
public:
    SensitivityInputReader() = default;
    SensitivityInputReader(const std::string &last_master_mps_path, const std::string &benders_out_json_path, const std::string &structure_file_path, double epsilon);
    ~SensitivityInputReader() = default;

    SolverAbstract::Ptr read_last_master();
    double read_best_ub();
    std::map<int, std::string> get_id_to_name();

private:
    std::string _last_master_path;
    std::string _structure_file_path;
    double _epsilon;
    Json::Value _json_data;

    Json::Value read_json(const std::string &json_file_path);
};
