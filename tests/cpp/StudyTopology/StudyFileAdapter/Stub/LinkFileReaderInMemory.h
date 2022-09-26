//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <string>
#include <vector>

#include "../ILinkFileReader.h"
#include "../Link.h"

class LinkFileReaderInMemory : public StudyFileReader::ILinkFileReader {
 public:
  virtual ~LinkFileReaderInMemory() = default;
  [[nodiscard]] std::vector<StudyFileReader::Link> Links(
      const std::string &study_path) const override;
};
