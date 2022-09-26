//
// Created by marechaljas on 20/09/22.
//

#include "Link.h"

#include "Candidate.h"

namespace XpansionStudy {
std::vector<Candidate> Link::Candidates() const {
  return candidates_;
}
void Link::AddCandidate(const Candidate& candidate) {
  candidates_.push_back(candidate);
}
}
