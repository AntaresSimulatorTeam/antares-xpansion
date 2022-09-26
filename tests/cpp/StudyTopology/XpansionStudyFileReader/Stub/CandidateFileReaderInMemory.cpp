//
// Created by marechaljas on 26/09/22.
//

#include "CandidateFileReaderInMemory.h"

void CandidateFileReaderInMemory::Feed(const StudyFileReader::Candidate &candidate) {
  candidates_.push_back(candidate);
}
std::vector<StudyFileReader::Candidate> CandidateFileReaderInMemory::Candidates(
    const std::string &study_path) const {
  return candidates_;
}
