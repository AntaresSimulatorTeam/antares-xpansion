//
// Created by marechaljas on 26/09/22.
//

#include "Adapter.h"

Adapter::Adapter(const StudyFileReader::ILinkFileReader &link_file_reader) {}
Adapter::Adapter(const StudyFileReader::IAreaFileReader &area_file_reader) {}

std::vector<Link> Adapter::Links(const std::string &study_path) const {
  return {};
}

std::vector<Area> Adapter::Areas(const std::string &study_path) const {
  return {};
}
