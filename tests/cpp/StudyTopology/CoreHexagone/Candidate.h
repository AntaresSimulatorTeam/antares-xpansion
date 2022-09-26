//
// Created by marechaljas on 23/09/22.
//

#pragma once

namespace XpansionStudy {
class Candidate {
 public:
  Candidate() = default;
  Candidate(const Candidate& candidate) = default;
  ~Candidate() = default;
  bool operator==(const Candidate& candidate) const;
};
}  // namespace XpansionStudy
