//
// Created by Archfiend on 14/03/2022.
//

#ifndef ANTARESXPANSION_PROJECTIONANALYSIS_H
#define ANTARESXPANSION_PROJECTIONANALYSIS_H
#include "Analysis.h"
#include "SensitivityILogger.h"
#include "SensitivityInputReader.h"
#include "SensitivityStudy.h"
class ProjectionAnalysis : public Analysis {
 public:
  ProjectionAnalysis(SensitivityInputData input_data,
                     std::string candidate_name,
                     std::shared_ptr<SensitivityILogger> logger);
  virtual std::pair<SinglePbData, SinglePbData> run();
};

#endif  // ANTARESXPANSION_PROJECTIONANALYSIS_H
