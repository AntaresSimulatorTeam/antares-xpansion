//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <vector>

#include "Candidate.h"

namespace XpansionStudy {
class Link {
 public:
  [[nodiscard]] std::vector<Candidate> Candidates() const;

  void AddCandidate(const XpansionStudy::Candidate& candidate);

 private:
  std::vector<Candidate> candidates_;
};
}  // namespace XpansionStudy