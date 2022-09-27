//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <string>
#include <vector>

#include "Model/Xpansion/Link.h"
#include "Model/Xpansion/Study.h"

class IForProvidingXpansionStudyPort {
 public:
  IForProvidingXpansionStudyPort() = default;
  IForProvidingXpansionStudyPort(const IForProvidingXpansionStudyPort&) =
      default;
  virtual ~IForProvidingXpansionStudyPort() = default;

  [[nodiscard]] virtual XpansionStudy::Study Study(
      const std::string& study_path) const = 0;
};
