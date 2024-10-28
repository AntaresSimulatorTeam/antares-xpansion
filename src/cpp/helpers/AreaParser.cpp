#include "antares-xpansion/helpers/AreaParser.h"

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

AreaFileData AreaParser::ReadAreaFile(const std::filesystem::path &areaFile) {
  std::ifstream area_filestream(areaFile);

  if (!area_filestream.good()) {
    std::ostringstream out_message("");
    out_message << LOGLOCATION << "unable to open " << areaFile.string();
    return {.error_message = out_message.str(), .areas = {}};
  }

  return {.error_message = "", .areas = ReadLineByLineArea(area_filestream)};
}

std::vector<std::string> AreaParser::ReadAreaFile(
    std::istringstream &areaFileInStringStream) {
  return ReadLineByLineArea(areaFileInStringStream);
}

std::vector<std::string> AreaParser::ReadLineByLineArea(std::istream &stream) {
  std::vector<std::string> result;

  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.front() != '#') {
      result.push_back(StringManip::StringUtils::ToLowercase(line));
    }
  }
  return result;
}
