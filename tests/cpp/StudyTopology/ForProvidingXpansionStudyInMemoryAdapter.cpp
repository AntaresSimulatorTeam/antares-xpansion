//
// Created by marechaljas on 20/09/22.
//

#include "ForProvidingXpansionStudyInMemoryAdapter.h"

void ForProvidingXpansionStudyInMemoryAdapter::addLink(Link link) {
  links_.push_back(link);
}
void ForProvidingXpansionStudyInMemoryAdapter::addCandidate(Candidate candidate) {
  candidates_.push_back(candidate);
}
XpansionStudy ForProvidingXpansionStudyInMemoryAdapter::Study() const {
  XpansionStudy xpansion_study;
  for (auto link: links_)
    xpansion_study.addLink(link);
  return xpansion_study;
}
