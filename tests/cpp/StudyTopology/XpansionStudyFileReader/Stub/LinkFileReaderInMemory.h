//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <string>
#include <vector>

#include "../../CoreHexagone/Model/Link.h"
#include "../ILinkFileReader.h"
#include "../Model/Link.h"

class LinkFileReaderInMemory : public StudyFileReader::ILinkFileReader {
 public:
  virtual ~LinkFileReaderInMemory() = default;
  [[nodiscard]] std::vector<StudyFileReader::Link> Links(
      const std::string &study_path) const override;
  void Feed(StudyFileReader::Link link);
  std::vector<StudyFileReader::Link> links_;
};
