//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include "../CoreHexagone/Link.h"
#include "../StudyFileAdapter/ILinkFileReader.h"
class Adapter {
 public:
  explicit Adapter(const StudyFileReader::ILinkFileReader &link_file_reader);
  [[nodiscard]] std::vector<Link> Links(const std::string &study_path) const;
};
