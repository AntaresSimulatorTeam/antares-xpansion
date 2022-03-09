#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include "SensitivityILogger.h"
#include "SensitivityInputReader.h"
#include "SensitivityOutputData.h"
#include "SensitivityPbModifier.h"
#include "SensitivityWriter.h"

class SensitivityAnalysis {
 public:
  SensitivityAnalysis() = default;
  explicit SensitivityAnalysis(const SensitivityInputData &input_data,
                               std::shared_ptr<SensitivityILogger> logger,
                               std::shared_ptr<SensitivityWriter> writer);
  ~SensitivityAnalysis() = default;

  static const bool MINIMIZE;
  static const bool MAXIMIZE;
  static const std::vector<std::string> sensitivity_string_pb_type;

  void launch();
  SensitivityOutputData get_output_data() const;

 private:
  double _epsilon;
  double _best_ub;

  bool _capex;
  std::vector<std::string> _projection;

  std::map<std::string, int> _name_to_id;
  SolverAbstract::Ptr _last_master;
  std::shared_ptr<SensitivityILogger> _logger;
  std::shared_ptr<SensitivityWriter> _writer;

  std::shared_ptr<SensitivityPbModifier> _pb_modifier;
  SensitivityOutputData _output_data;

  SensitivityPbType _sensitivity_pb_type;

  void init_output_data();
  SinglePbData init_single_pb_data(const bool minimize) const;
  void get_capex_solutions();
  void get_candidates_projection();
  void run_analysis();
  void run_optimization(const SolverAbstract::Ptr &sensitivity_model,
                        const bool minimize);

  RawPbData solve_sensitivity_pb(SolverAbstract::Ptr sensitivity_problem) const;
  void fill_single_pb_data(SinglePbData &pb_data,
                           const RawPbData &raw_output) const;
  double get_system_cost(const RawPbData &raw_output) const;
};