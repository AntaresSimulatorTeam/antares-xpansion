#pragma once

#include <iostream>
#include "common.h"

struct SinglePbData
{
    std::string pb_type;
    std::string opt_dir;
    double objective;    // should contain specific pb objective for each sensitivity pb (either min/max candidate cost or capex min/max)
    double system_cost;  // System cost (i.e. invest. cost + operational cost) of the solution of the sensitivity problem
    Point candidates;
    int status;
};

struct SensitivityOutputData
{
    double epsilon;
    double best_benders_cost;
    std::vector<SinglePbData> pbs_data;
};

struct RawPbData
{
    double status;
    double obj_value;
    std::vector<double> obj_coeffs;
    std::vector<double> solution;
};