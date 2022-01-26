#pragma once

#include <iostream>
#include "common.h"

struct SensitivityOutputData
{
    double epsilon;
    double best_benders_cost; //ou stocker un objet Output::SolutionData
    double solution_system_cost;  // System cost (i.e. invest. cost + operational cost) of the solution of the sensitivity problem
    double pb_objective; // should contain specific pb objective for each sensitivity pb (either min/max candidate cost or capex min/max)
    Point candidates;
    int pb_status;
};