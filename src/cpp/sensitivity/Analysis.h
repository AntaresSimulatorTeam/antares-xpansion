#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__
#include "SensitivityILogger.h"
#include "SensitivityInputReader.h"
#include "SensitivityStudy.h"

class Analysis {
 public:
  Analysis(SensitivityInputData input_data, std::string candidate_name,
           std::shared_ptr<SensitivityILogger> logger,
           const SensitivityPbType type);
  std::pair<SinglePbData, SinglePbData> run();

 protected:
  SinglePbData run_optimization(SensitivityStudy::StudyType minimize);

 private:
  const SensitivityInputData input_data;
  const std::string candidate_name;
  const std::shared_ptr<SensitivityILogger> logger;
  std::shared_ptr<SolverAbstract> sensitivity_pb_model;
  SensitivityPbType problem_type;

  void fill_single_pb_data(SinglePbData& pb_data,
                           const RawPbData& raw_output) const;
  double get_system_cost(const RawPbData& raw_output) const;
};

#endif  //__ANALYSIS_H__