//
// Created by marechaljas on 20/09/22.
//

#include "StudyInMemoryAdapter.h"

void StudyInMemoryAdapter::addLink(XpansionStudy::Link link) {
  links_.push_back(link);
}
void StudyInMemoryAdapter::addCandidate(
    XpansionStudy::Candidate candidate) {
  candidates_.push_back(candidate);
}
XpansionStudy::Study StudyInMemoryAdapter::Study(const std::string& study_path) const {
  auto copy = links_;
  return { std::move(copy) };
}
