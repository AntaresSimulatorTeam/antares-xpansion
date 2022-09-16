//
// Created by marechaljas on 16/09/22.
//

#pragma once

#include <filesystem>
#include <vector>
class IntercoFileReader {
 public:

  struct IntercoFileData {
    int index_interco;
    int index_pays_origine;
    int index_pays_extremite;
  };

  static std::vector<IntercoFileData> ReadAntaresIntercoFile(
      const std::filesystem::path& antaresIntercoFile);
};
