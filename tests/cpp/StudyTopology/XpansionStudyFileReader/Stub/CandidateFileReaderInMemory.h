//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <vector>

#include "../ICandidateFileReader.h"
#include "../Model/Candidate.h"

class CandidateFileReaderInMemory : public StudyFileReader::ICandidateFileReader {
 public:
  virtual ~CandidateFileReaderInMemory() = default;
  void Feed(const StudyFileReader::Candidate &candidate);
  [[nodiscard]] std::vector<StudyFileReader::Candidate> Candidates(
      const std::string &study_path) const override;
  std::vector<StudyFileReader::Candidate> candidates_;
};
