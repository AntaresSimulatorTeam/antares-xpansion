//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <string>

#include "Model/Candidate.h"

namespace StudyFileReader {
class ICandidateFileReader {
 public:
  [[nodiscard]] virtual std::vector<Candidate> Candidates(
      const std::string& study_path) const = 0;
};
}  // namespace StudyFileReader
