//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <vector>
#include <string>

#include "../CoreHexagone/Model/Link.h"
#include "../CoreHexagone/Model/Study.h"

class IStudyAdapter {
 public:
  IStudyAdapter() = default;
  IStudyAdapter(const IStudyAdapter&) = default;
  virtual ~IStudyAdapter() = default;

  [[nodiscard]] virtual XpansionStudy::Study Study(const std::string& study_path) const = 0;
};
