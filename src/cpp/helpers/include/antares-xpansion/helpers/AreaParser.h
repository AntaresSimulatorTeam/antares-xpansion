#pragma once
#include <filesystem>
#include <fstream>
#include <vector>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

struct AreaFileError : LogUtils::XpansionError<std::ios_base::failure> {
  using LogUtils::XpansionError<std::ios_base::failure>::XpansionError;
};
struct AreaFileData {
  std::string error_message = "";
  std::vector<std::string> areas;
};

struct AreaParser {
  static AreaFileData ReadAreaFile(const std::filesystem::path& areaFile);
  static std::vector<std::string> ReadAreaFile(
      std::istringstream& areaFileInStringStream);
  static std::vector<std::string> ReadLineByLineArea(std::istream& stream);
};