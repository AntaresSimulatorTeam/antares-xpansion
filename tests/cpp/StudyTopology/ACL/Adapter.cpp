//
// Created by marechaljas on 26/09/22.
//

#include "Adapter.h"

Adapter::Adapter(const StudyFileReader::ILinkFileReader &link_file_reader) {}
Adapter::Adapter(const StudyFileReader::IAreaFileReader &area_file_reader) {}
Adapter::Adapter(
    const StudyFileReader::ICandidateFileReader &candidate_reader) {

}
Adapter::Adapter(LinkFileReaderInMemory link_reader,
                 AreaFileReaderInMemory area_reader,
                 CandidateFileReaderInMemory candidate_reader) {

}

std::vector<Link> Adapter::Links(const std::string &study_path) const {
  return {};
}

std::vector<Area> Adapter::Areas(const std::string &study_path) const {
  return {};
}
std::vector<Candidate> Adapter::Candidates(const std::string &study_path) {
  return {};
}
XpansionStudy::Study Adapter::Study(const std::string &study_path) {
  return {};
}

