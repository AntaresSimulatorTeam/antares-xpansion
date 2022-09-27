//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <string>
#include <vector>

#include "Model/Link.h"
#include "Model/Study.h"

class IForProvidingStudyPort {
 public:
  IForProvidingStudyPort() = default;
  IForProvidingStudyPort(const IForProvidingStudyPort&) = default;
  virtual ~IForProvidingStudyPort() = default;

  [[nodiscard]] virtual XpansionStudy::Study Study(const std::string& study_path) const = 0;
};
