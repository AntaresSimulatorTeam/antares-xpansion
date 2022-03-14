#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include "SensitivityILogger.h"
#include "SensitivityInputReader.h"
#include "SensitivityOutputData.h"
#include "SensitivityProblemModifier.h"
#include "SensitivityWriter.h"

class SensitivityStudy {
 public:
  SensitivityStudy() = delete;
  explicit SensitivityStudy(const SensitivityInputData &input_data,
                               std::shared_ptr<SensitivityILogger> logger,
                               std::shared_ptr<SensitivityWriter> writer);
  ~SensitivityStudy() = default;

  enum class StudyType: bool {
    MINIMIZE = true,
    MAXIMIZE = false,
  };

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
  std::shared_ptr<SensitivityILogger> logger;
  std::shared_ptr<SensitivityWriter> writer;
  SensitivityInputData input_data;

  std::shared_ptr<SensitivityProblemModifier> _pb_modifier;
  SensitivityOutputData _output_data;

  SensitivityPbType _sensitivity_pb_type;

  void init_output_data();
  SinglePbData init_single_pb_data(StudyType minimize) const;
  void run_capex_analysis();
  void get_candidates_projection();
  void run_analysis();
  void run_optimization(const SolverAbstract::Ptr &sensitivity_model,
                        StudyType minimize);

  void fill_single_pb_data(SinglePbData &pb_data,
                           const RawPbData &raw_output) const;
  double get_system_cost(const RawPbData &raw_output) const;
};