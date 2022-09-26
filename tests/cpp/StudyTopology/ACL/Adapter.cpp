//
// Created by marechaljas on 26/09/22.
//

#include "Adapter.h"

#include <utility>

Adapter::Adapter(const StudyFileReader::ILinkFileReader &link_file_reader) {}
Adapter::Adapter(const StudyFileReader::IAreaFileReader &area_file_reader) {}
Adapter::Adapter(
    const StudyFileReader::ICandidateFileReader &candidate_reader) {

}
Adapter::Adapter(std::shared_ptr<StudyFileReader::ILinkFileReader> link_reader,
                 std::shared_ptr<StudyFileReader::IAreaFileReader> area_reader,
                 std::shared_ptr<StudyFileReader::ICandidateFileReader> candidate_reader,
                 std::shared_ptr<ILinkTranslator> link_translator)
    : link_reader_(std::move(link_reader)),
      area_reader_(std::move(area_reader)),
      candidate_reader_(std::move(candidate_reader)),
      link_translator_(std::move(link_translator))
{

}

std::vector<XpansionStudy::Link> Adapter::Links(const std::string &study_path) const {
  return {};
}

std::vector<XpansionStudy::Area> Adapter::Areas(const std::string &study_path) const {
  return {};
}
std::vector<XpansionStudy::Candidate> Adapter::Candidates(const std::string &study_path) {
  return {};
}
XpansionStudy::Study Adapter::Study(const std::string &study_path) {
  auto links = link_reader_->Links(study_path);
  auto candidates = candidate_reader_->Candidates(study_path);
  auto study_links = link_translator_->translate(links, candidates);

  return {std::move(study_links)};
}

