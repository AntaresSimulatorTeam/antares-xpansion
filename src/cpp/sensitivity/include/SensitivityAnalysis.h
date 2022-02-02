#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityWriter.h"
#include "SensitivityOutputData.h"
#include "SensitivityPbModifier.h"

enum SensitivityPbType
{
    CAPEX,
    PROJECTION,
};

class SensitivityAnalysis
{
public:
    SensitivityAnalysis() = default;
    explicit SensitivityAnalysis(double epsilon, double best_ub,
                                 const std::map<int, std::string> &id_to_name,
                                 SolverAbstract::Ptr last_master, std::shared_ptr<SensitivityWriter> writer);
    ~SensitivityAnalysis() = default;

    static const bool MINIMIZE;
    static const bool MAXIMIZE;
    static const std::vector<std::string> sensitivity_string_pb_type;

    void launch();
    SensitivityOutputData get_output_data() const;

private:
    double _epsilon;
    double _best_ub;

    std::map<int, std::string> _id_to_name;
    SolverAbstract::Ptr _last_master;
    std::shared_ptr<SensitivityWriter> _writer;

    std::shared_ptr<SensitivityPbModifier> _pb_modifier;
    SensitivityOutputData _output_data;

    SensitivityPbType _sensitivity_pb_type;

    void init_output_data();
    void run_analysis();
    void run_optimization(const SolverAbstract::Ptr &sensitivity_model, const bool minimize);

    void get_capex_solutions();
    void get_candidates_projection();

    RawPbData solve_sensitivity_pb(SolverAbstract::Ptr sensitivity_problem);
    void fill_output_data(const RawPbData &raw_output, const bool minimize);
};