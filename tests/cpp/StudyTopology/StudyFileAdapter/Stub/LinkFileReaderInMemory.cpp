//
// Created by marechaljas on 26/09/22.
//

#include "LinkFileReaderInMemory.h"

std::vector<StudyFileReader::Link> LinkFileReaderInMemory::Links(
    const std::string &study_path) const {
  return links_;
}
void LinkFileReaderInMemory::Feed(StudyFileReader::Link link) {
  links_.push_back(link);
}
