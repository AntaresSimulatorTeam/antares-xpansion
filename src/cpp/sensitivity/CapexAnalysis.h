
#ifndef ANTARESXPANSION_CAPEXANALYSIS_H
#define ANTARESXPANSION_CAPEXANALYSIS_H

#include <SensitivityILogger.h>
#include <SensitivityInputReader.h>
#include "SensitivityStudy.h"
class CapexAnalysis {
 public:
  CapexAnalysis(SensitivityInputData input_data,
                std::shared_ptr<SensitivityILogger> logger);

  std::pair<SinglePbData, SinglePbData>  run();

 private:
  std::shared_ptr<SensitivityILogger> logger;
  SensitivityInputData input_data;
  std::shared_ptr<SolverAbstract> sensitivity_pb_model;

  SinglePbData run_optimization(SensitivityStudy::StudyType minimize);
  RawPbData solve_sensitivity_pb() const;
  void fill_single_pb_data(SinglePbData &pb_data,
                           const RawPbData &raw_output) const;
  double get_system_cost(const RawPbData &raw_output) const;

};

#endif  // ANTARESXPANSION_CAPEXANALYSIS_H
