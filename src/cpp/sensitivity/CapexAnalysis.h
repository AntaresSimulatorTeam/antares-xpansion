
#ifndef ANTARESXPANSION_CAPEXANALYSIS_H
#define ANTARESXPANSION_CAPEXANALYSIS_H

#include <SensitivityILogger.h>
#include <SensitivityInputReader.h>
class CapexAnalysis {
 public:
  CapexAnalysis(const SensitivityInputData &input_data,
                std::shared_ptr<SensitivityILogger> logger);

  std::pair<SinglePbData, SinglePbData>  run();

 private:
  std::shared_ptr<SensitivityILogger> logger;
  SensitivityInputData input_data;

  void run_optimization(
      const SolverAbstract::Ptr &sensitivity_model, const bool minimize);
  SinglePbData init_single_pb_data(const bool minimize) const;
};

#endif  // ANTARESXPANSION_CAPEXANALYSIS_H
