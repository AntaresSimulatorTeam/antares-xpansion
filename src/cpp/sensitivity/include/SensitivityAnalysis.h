#pragma once

#include <multisolver_interface/SolverAbstract.h>
#include "SensitivityInputReader.h"
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
    explicit SensitivityAnalysis(const SensitivityInputData &input_data, std::shared_ptr<SensitivityWriter> writer);
    ~SensitivityAnalysis() = default;

    static const bool MINIMIZE;
    static const bool MAXIMIZE;
    static const std::vector<std::string> sensitivity_string_pb_type;

    void launch();
    void get_capex_solutions();
    void get_candidates_projection();
    SensitivityOutputData get_output_data() const;

private:
    double _epsilon;
    double _best_ub;

    bool _capex;
    std::vector<std::string> _projection;

    std::map<int, std::string> _id_to_name;
    SolverAbstract::Ptr _last_master;
    std::shared_ptr<SensitivityWriter> _writer;

    std::shared_ptr<SensitivityPbModifier> _pb_modifier;
    SensitivityOutputData _output_data;

    SensitivityPbType _sensitivity_pb_type;

    void init_output_data();
    void run_analysis();
    void run_optimization(const SolverAbstract::Ptr &sensitivity_model, const bool minimize);

    RawPbData solve_sensitivity_pb(SolverAbstract::Ptr sensitivity_problem);
    void fill_output_data(const RawPbData &raw_output, const bool minimize);
    double get_system_cost(const RawPbData &raw_output);
    std::string get_projection_pb_candidate_name(const RawPbData &raw_output);
};