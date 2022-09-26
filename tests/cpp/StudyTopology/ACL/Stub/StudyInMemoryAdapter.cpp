//
// Created by marechaljas on 20/09/22.
//

#include "StudyInMemoryAdapter.h"

void StudyInMemoryAdapter::addLink(Link link) {
  links_.push_back(link);
}
void StudyInMemoryAdapter::addCandidate(Candidate candidate) {
  candidates_.push_back(candidate);
}
XpansionStudy StudyInMemoryAdapter::Study() const {
  XpansionStudy xpansion_study;
  for (auto link: links_)
    xpansion_study.addLink(link);
  return xpansion_study;
}
