#pragma once

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

#include "antares-xpansion/sensitivity/SensitivityILogger.h"
#include "antares-xpansion/sensitivity/SensitivityInputReader.h"
#include "antares-xpansion/sensitivity/SensitivityOutputData.h"
#include "antares-xpansion/sensitivity/SensitivityProblemModifier.h"
#include "antares-xpansion/sensitivity/SensitivityWriter.h"

class SensitivityStudy {
 public:
  SensitivityStudy() = delete;
  explicit SensitivityStudy(SensitivityInputData input_data,
                               std::shared_ptr<SensitivityILogger> logger,
                               std::shared_ptr<SensitivityWriter> writer);
  ~SensitivityStudy() = default;

  enum class StudyType: bool {
    MINIMIZE = true,
    MAXIMIZE = false,
  };

  void launch();
  std::vector<SinglePbData> get_output_data() const;

 private:
  std::shared_ptr<SensitivityILogger> logger;
  std::shared_ptr<SensitivityWriter> writer;
  SensitivityInputData input_data;
  std::vector<SinglePbData> output_data;

  void run_capex_analysis();
  void run_projection_analysis();

};