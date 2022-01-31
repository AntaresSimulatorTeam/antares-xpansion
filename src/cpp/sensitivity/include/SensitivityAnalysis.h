#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityWriter.h"
#include "SensitivityPbModifier.h"
#include "SensitivityOutputData.h"

class SensitivityAnalysis
{
public:
    SensitivityAnalysis() = default;
    explicit SensitivityAnalysis(double epsilon, double bestUb,
                                 const std::map<int, std::string> &idToName,
                                 SolverAbstract::Ptr lastMaster, std::shared_ptr<SensitivityWriter> writer);
    ~SensitivityAnalysis() = default;

    void launch();
    SensitivityOutputData get_output_data() const;

private:
    double _epsilon;
    double _best_ub;

    std::map<int, std::string> _id_to_name;
    SolverAbstract::Ptr _last_master;
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

    std::vector<double> get_sensitivity_solution(SolverAbstract::Ptr sensitivity_problem);
    void fill_output_data(const std::vector<double> &solution);
};