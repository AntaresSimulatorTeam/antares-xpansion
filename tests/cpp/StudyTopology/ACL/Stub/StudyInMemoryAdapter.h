//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include "../../CoreHexagone/IForProvidingXpansionStudyPort.h"
#include "../../CoreHexagone/Model/Xpansion/Candidate.h"
#include "../../CoreHexagone/Model/Xpansion/Link.h"
#include "../../XpansionStudyFileReader/Model/Link.h"

class StudyInMemoryAdapter : public IForProvidingXpansionStudyPort {
 public:
  void addLink(XpansionStudy::Link link);
  void addCandidate(XpansionStudy::Candidate candidate);
  [[nodiscard]] XpansionStudy::Study Study(
      const std::string& study_path) const override;
  std::vector<XpansionStudy::Link> links_;
  std::vector<XpansionStudy::Candidate> candidates_;
};
