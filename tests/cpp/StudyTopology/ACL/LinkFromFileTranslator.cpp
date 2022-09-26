//
// Created by marechaljas on 26/09/22.
//

#include "LinkFromFileTranslator.h"

XpansionStudy::Link LinkFromFileTranslator::translate(const StudyFileReader::Link& link, const std::vector<StudyFileReader::Candidate>& candidates) const {
    return {};
}

std::vector<XpansionStudy::Link> LinkFromFileTranslator::translate(
    const std::vector<StudyFileReader::Link>& links,
    const std::vector<StudyFileReader::Candidate>& candidates) const {
  std::vector<XpansionStudy::Link> study_links;
  for (const auto link: links) {
    study_links.push_back(translate(link, candidates));
  }
  return study_links;
}
