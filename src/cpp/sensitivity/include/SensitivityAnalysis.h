#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityLogger.h"
#include "SensitivityWriter.h"

class SensitivityAnalysis
{
public:
    explicit SensitivityAnalysis(std::shared_ptr<SolverAbstract> &lastMasterProblem, SensitivityLogger &logger, SensitivityWriter writer);
    ~SensitivityAnalysis();

    void launch();

private:
    std::shared_ptr<SolverAbstract> _last_master_model;
    std::shared_ptr<SolverAbstract> _sensitivity_pb_model;

    SensitivityLogger _logger;
    SensitivityWriter _writer;

    void get_capex_optimal_solutions();
    void get_capex_min_solution();
    void get_capex_max_solution();

    void get_candidates_projection();
    void get_candidate_upper_projection(int &candidateNum);
    void get_candidate_lower_projection(int &candidateNum);

    void solve_sensitivity_pb();
};