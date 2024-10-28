#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__
#include "antares-xpansion/sensitivity/SensitivityILogger.h"
#include "antares-xpansion/sensitivity/SensitivityInputReader.h"
#include "antares-xpansion/sensitivity/SensitivityStudy.h"

class Analysis {
 public:
  Analysis(SensitivityInputData input_data, std::string candidate_name,
           std::shared_ptr<SensitivityILogger> logger, SensitivityPbType type);
  std::pair<SinglePbData, SinglePbData> run();

 protected:
  SinglePbData run_optimization(SensitivityStudy::StudyType minimize);

 private:
  const SensitivityInputData input_data;
  const std::string candidate_name;
  const std::shared_ptr<SensitivityILogger> logger;
  SensitivityPbType problem_type;
  std::shared_ptr<SolverAbstract> sensitivity_pb_model;

  void fill_single_pb_data(SinglePbData& pb_data,
                           const RawPbData& raw_output) const;
  [[nodiscard]] double get_system_cost(const RawPbData& raw_output) const;
  void get_sensitivity_problem();
  RawPbData solve_sensitivity_pb() const;
};

#endif  //__ANALYSIS_H__