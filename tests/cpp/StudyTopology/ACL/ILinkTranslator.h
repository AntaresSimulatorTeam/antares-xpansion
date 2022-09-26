//
// Created by marechaljas on 26/09/22.
//

#pragma once

class ILinkTranslator {
 public:
  [[nodiscard]] virtual std::vector<XpansionStudy::Link> translate(
      const std::vector<StudyFileReader::Link> &links,
      const std::vector<StudyFileReader::Candidate> &candidates) const = 0;
};
