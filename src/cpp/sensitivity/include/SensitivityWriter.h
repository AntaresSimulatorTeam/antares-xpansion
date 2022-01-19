#pragma once

#include <json/writer.h>

struct SensitivityCandidate
{
    std::string name;
    double invest;
};

struct SensitivityOutputData
{
    double epsilon;
    double best_overall_cost; //ou stocker un objet Output::SolutionData
    double sensitivity_solution_overall_cost;
    double sensitivity_pb_objective; // should contain specific pb objective for each sensitivity pb (either min/max candidate cost or cpaex min/max)
    std::vector<SensitivityCandidate> sensitivity_candidates;
};

class SensitivityWriter
{
private:
    std::string _filename;
    Json::Value _output;

public:
    SensitivityWriter() = delete;
    SensitivityWriter(const std::string &json_filename);
    ~SensitivityWriter() = default;

    void dump();
    void write_sensitivity_output(SensitivityOutputData const &output_data);
    void end_writing(SensitivityOutputData const &output_data);
};