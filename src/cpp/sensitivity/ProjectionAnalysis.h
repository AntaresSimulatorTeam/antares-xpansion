//
// Created by Archfiend on 14/03/2022.
//

#ifndef ANTARESXPANSION_PROJECTIONANALYSIS_H
#define ANTARESXPANSION_PROJECTIONANALYSIS_H

#include "SensitivityInputReader.h"
#include "SensitivityILogger.h"
#include "SensitivityStudy.h"
class ProjectionAnalysis {
 public:
  ProjectionAnalysis(SensitivityInputData input_data, std::string candidate_name,
                     std::shared_ptr<SensitivityILogger> logger);
  std::pair<SinglePbData, SinglePbData> run();

 private:
  const std::shared_ptr<SensitivityILogger> logger;
  const SensitivityInputData input_data;
  const std::string candidate_name;
  std::shared_ptr<SolverAbstract> sensitivity_pb_model;

  SinglePbData run_optimization(SensitivityStudy::StudyType minimize);
  void fill_single_pb_data(SinglePbData& pb_data,
                           const RawPbData& raw_output) const;
  double get_system_cost(const RawPbData& raw_output) const;
};

#endif  // ANTARESXPANSION_PROJECTIONANALYSIS_H
