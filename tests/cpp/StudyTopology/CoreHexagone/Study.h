//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <vector>

#include "Area.h"
#include "Candidate.h"
#include "Link.h"
namespace XpansionStudy {

class Study {
 public:
  [[nodiscard]] std::vector<Area> Areas() const;
  [[nodiscard]] std::vector<Link> Links() const;
  [[nodiscard]] std::vector<Candidate> Candidates() const;
};

}  // namespace XpansionStudy
