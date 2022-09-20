//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <vector>

#include "Link.h"
class IStudyAdapter {
 public:
  [[nodiscard]] virtual std::vector<Link> Links() const = 0;
};
