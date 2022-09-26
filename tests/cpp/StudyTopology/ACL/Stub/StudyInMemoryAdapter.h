//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include "../../CoreHexagone/Candidate.h"
#include "../../CoreHexagone/Link.h"
#include "../../StudyFileAdapter/Link.h"
#include "../IStudyAdapter.h"

class StudyInMemoryAdapter : public IStudyAdapter {
 public:
  void addLink(XpansionStudy::Link link);
  void addCandidate(XpansionStudy::Candidate candidate);
  XpansionStudy::XpansionStudy Study() const override;
  std::vector<XpansionStudy::Link> links_;
  std::vector<XpansionStudy::Candidate> candidates_;
};
