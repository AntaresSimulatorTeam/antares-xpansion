//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include "../../CoreHexagone/Candidate.h"
#include "../../CoreHexagone/Link.h"
#include "../IStudyAdapter.h"
class StudyInMemoryAdapter : public IStudyAdapter {
 public:
  void addLink(Link link);
  void addCandidate(Candidate candidate);
  XpansionStudy Study() const override;
  std::vector<Link> links_;
  std::vector<Candidate> candidates_;
};
