//
// Created by marechaljas on 16/09/22.
//

#pragma once

#include <filesystem>
#include <vector>
class AreaFileReader {
 public:
  static std::vector<std::string> ReadAreaFile(
      const std::filesystem::path& areaFile);
};
