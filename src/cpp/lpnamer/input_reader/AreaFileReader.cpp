//
// Created by marechaljas on 16/09/22.
//

#include "AreaFileReader.h"

#include <fstream>

#include "helpers/StringUtils.h"
std::vector<std::string> AreaFileReader::ReadAreaFile(
    const std::filesystem::path& areaFile) {
  std::vector<std::string> result;

  std::ifstream area_filestream(areaFile);
  if (!area_filestream.good()) {
    std::string message = "unable to open " + areaFile.string();
    throw std::runtime_error(message);
  }

  std::string line;
  while (std::getline(area_filestream, line)) {
    if (!line.empty() && line.front() != '#') {
      result.push_back(StringUtils::ToLowercase(line));
    }
  }
  return result;
}
