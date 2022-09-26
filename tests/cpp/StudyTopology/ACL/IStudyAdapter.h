//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <vector>

#include "../CoreHexagone/Link.h"
#include "../CoreHexagone/XpansionStudy.h"
class IStudyAdapter {
 public:
  IStudyAdapter() = default;
  IStudyAdapter(const IStudyAdapter&) = default;
  virtual ~IStudyAdapter() = default;

  [[nodiscard]] virtual XpansionStudy Study() const = 0;
};
