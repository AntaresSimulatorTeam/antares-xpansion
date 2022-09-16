//
// Created by marechaljas on 16/09/22.
//

#include "IntercoFileReader.h"

#include <fstream>
std::vector<IntercoFileReader::IntercoFileData> IntercoFileReader::ReadAntaresIntercoFile(
    const std::filesystem::path& antaresIntercoFile) {
  std::vector<IntercoFileData> result;

  std::ifstream interco_filestream(antaresIntercoFile);
  if (!interco_filestream.good()) {
    std::string message = "unable to open " + antaresIntercoFile.string();
    throw std::runtime_error(message);
  }

  std::string line;
  while (std::getline(interco_filestream, line)) {
    std::stringstream buffer(line);
    if (!line.empty() && line.front() != '#') {
      IntercoFileData intercoData;
      buffer >> intercoData.index_interco;
      buffer >> intercoData.index_pays_origine;
      buffer >> intercoData.index_pays_extremite;

      result.push_back(intercoData);
    }
  }
  return result;
}
