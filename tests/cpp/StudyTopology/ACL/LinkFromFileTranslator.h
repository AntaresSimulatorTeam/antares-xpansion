//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <vector>

#include "../CoreHexagone/Candidate.h"
#include "../CoreHexagone/Link.h"
#include "../StudyFileAdapter/Candidate.h"
#include "../StudyFileAdapter/Link.h"
#include "ILinkTranslator.h"
class LinkFromFileTranslator : public ILinkTranslator {
 public:
  virtual ~LinkFromFileTranslator() = default;
  [[nodiscard]] std::vector<XpansionStudy::Link> translate(
      const std::vector<StudyFileReader::Link> &links,
      const std::vector<StudyFileReader::Candidate> &candidates) const override;
 private:
  [[nodiscard]] XpansionStudy::Link translate(const StudyFileReader::Link& link,
                                const std::vector<StudyFileReader::Candidate>& candidates) const;
};
