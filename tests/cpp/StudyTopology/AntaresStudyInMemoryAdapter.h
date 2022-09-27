//
// Created by marechaljas on 27/09/22.
//

#pragma once

#include <string>

#include "CoreHexagone/IForProvidingAntaresStudyPort.h"
#include "CoreHexagone/Model/Antares/Study.h"
class AntaresStudyInMemoryAdapter : public IForProvidingAntaresStudyPort {
 public:
  virtual ~AntaresStudyInMemoryAdapter() = default;
  [[nodiscard]] AntaresStudy::Study Study(
      const std::string& study_path) const override;
};
