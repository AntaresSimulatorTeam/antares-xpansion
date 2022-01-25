#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityWriter.h"
#include "SensitivityPbModifier.h"
#include "SensitivityOutputData.h"

class SensitivityAnalysis
{
public:
    explicit SensitivityAnalysis(double epsilon, double bestUb, std::shared_ptr<SolverAbstract> lastMasterProblem,
                                 std::map<int, std::string> idToName, std::shared_ptr<SensitivityWriter> writer);
    ~SensitivityAnalysis();

    void launch();

private:
    double _epsilon;
    double _best_ub;

    std::map<int, std::string> _id_to_name;
    std::shared_ptr<SolverAbstract> _sensitivity_pb_model;
    std::shared_ptr<SensitivityWriter> _writer;

    SensitivityPbModifier _pb_modifier;
    SensitivityOutputData _output_data;

    void init_output_data();

    // void get_capex_optimal_solutions();
    void get_capex_min_solution();
    // void get_capex_max_solution();

    // void get_candidates_projection();
    // void get_candidate_upper_projection(int &candidateNum);
    // void get_candidate_lower_projection(int &candidateNum);

    void solve_sensitivity_pb();
};