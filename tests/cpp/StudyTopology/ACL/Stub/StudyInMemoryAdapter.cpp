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
XpansionStudy::XpansionStudy StudyInMemoryAdapter::Study() const {
  XpansionStudy::XpansionStudy xpansion_study;
  for (auto link: links_)
    xpansion_study.addLink(link);
  return xpansion_study;
}
