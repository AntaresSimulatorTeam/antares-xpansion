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
  Study(std::vector<Link>&& links);
  [[nodiscard]] std::vector<Area> Areas() const;
  [[nodiscard]] std::vector<Link> Links() const;
  [[nodiscard]] std::vector<Candidate> Candidates() const;
 private:
  std::vector<Link> links_;
};

}  // namespace XpansionStudy
