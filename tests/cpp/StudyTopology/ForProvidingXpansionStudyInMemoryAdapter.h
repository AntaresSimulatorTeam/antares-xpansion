//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include "Candidate.h"
#include "IStudyAdapter.h"
#include "Link.h"
class ForProvidingXpansionStudyInMemoryAdapter : public IStudyAdapter {
 public:
  void addLink(Link link);
  void addCandidate(Candidate candidate);
  XpansionStudy Study() const override;
  std::vector<Link> links_;
  std::vector<Candidate> candidates_;
};
