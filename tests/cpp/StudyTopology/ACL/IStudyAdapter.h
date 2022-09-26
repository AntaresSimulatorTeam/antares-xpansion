//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <vector>

#include "../CoreHexagone/Model/Link.h"
#include "../CoreHexagone/Model/XpansionStudy.h"
class IStudyAdapter {
 public:
  IStudyAdapter() = default;
  IStudyAdapter(const IStudyAdapter&) = default;
  virtual ~IStudyAdapter() = default;

  [[nodiscard]] virtual XpansionStudy::XpansionStudy Study() const = 0;
};
