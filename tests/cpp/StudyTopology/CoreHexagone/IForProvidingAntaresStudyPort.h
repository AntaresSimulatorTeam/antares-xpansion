//
// Created by marechaljas on 27/09/22.
//

#pragma once

#include <string>

#include "Model/Antares/Study.h"
class IForProvidingAntaresStudyPort {
 public:
  [[nodiscard]] virtual AntaresStudy::Study Study(
      const std::string& study_path) const = 0;
};
