//
// Created by marechaljas on 03/06/22.
//

#include "antares-xpansion/helpers/AntaresVersionProvider.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

const std::string STUDY_FILE{"study.antares"};

int AntaresVersionProvider::getAntaresVersion(
    const std::filesystem::path& study_path) const {
  std::ifstream studyFile_l(study_path / STUDY_FILE);
  std::string line_l;
  const std::string versionProperty_l("version = ");

  int version = DEFAULT_ANTARES_VERSION;
  while (std::getline(studyFile_l, line_l)) {
    size_t pos_l = line_l.find(versionProperty_l);
    if (pos_l != std::string::npos) {
      std::stringstream buffer(line_l.substr(pos_l + versionProperty_l.size()));
      buffer >> version;
      if (buffer.fail()) {
          version = DEFAULT_ANTARES_VERSION;
      }
      return version;
    }
  }

  // default
  std::cout
      << "AntaresVersionProvider::getAntaresVersion : default version 710 returned.\n";
  return version;
}
