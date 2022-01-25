#pragma once

#include <iostream>
#include "common.h"

struct SensitivityOutputData
{
    double epsilon;
    double best_benders_cost; //ou stocker un objet Output::SolutionData
    double sensitivity_solution_overall_cost;
    double sensitivity_pb_objective; // should contain specific pb objective for each sensitivity pb (either min/max candidate cost or cpaex min/max)
    Point sensitivity_candidates;
    int sensitivity_pb_status;
};