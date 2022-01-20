#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityLogger.h"
#include "SensitivityWriter.h"

class SensitivityAnalysis
{
public:
    explicit SensitivityAnalysis(double epsilon, std::shared_ptr<SolverAbstract> &lastMasterProblem, std::shared_ptr<SensitivityWriter> writer);
    ~SensitivityAnalysis();

    void launch();

private:
    double _epsilon;

    std::shared_ptr<SolverAbstract> _last_master_model;
    std::shared_ptr<SolverAbstract> _sensitivity_pb_model;

    std::shared_ptr<SensitivityWriter> _writer;

    // void get_capex_optimal_solutions();
    SensitivityOutputData get_capex_min_solution();
    // void get_capex_max_solution();

    // void get_candidates_projection();
    // void get_candidate_upper_projection(int &candidateNum);
    // void get_candidate_lower_projection(int &candidateNum);

    SensitivityOutputData solve_sensitivity_pb();
};