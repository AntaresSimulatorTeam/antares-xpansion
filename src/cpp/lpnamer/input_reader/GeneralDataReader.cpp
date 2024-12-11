#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/Exceptions.h"
#include "antares-xpansion/lpnamer/input_reader/INIReader.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"
class IniReaderUtils {
 public:
  static bool LineIsNotASectionHeader(const std::string& line) {
    return StringManip::split(StringManip::trim(line), '=').size() == 2;
  }

  static std::string ReadSectionHeader(const std::string& line) {
    auto str = StringManip::trim(line);
    str.erase(std::remove(str.begin(), str.end(), '['), str.end());
    str.erase(std::remove(str.begin(), str.end(), ']'), str.end());
    return str;
  }

  static std::pair<std::string, int> GetKeyValFromLine(std::string_view line) {
    const auto splitLine = StringManip::split(line, '=');
    auto key = StringManip::trim(splitLine[0]);
    auto val = std::stoi(StringManip::trim(splitLine[1]));
    return {key, val};
  }
};
GeneralDataIniReader::GeneralDataIniReader(
    const std::filesystem::path& file_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : logger_(logger) {
  if (file_path.empty() || !std::filesystem::exists(file_path)) {
    std::ostringstream msg;
    msg << LOGLOCATION
        << "General data file is not found : " << file_path.string()
        << std::endl;
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << msg.str();
    throw IniFileNotFound(msg.str());
  }

  reader_ = INIReader(file_path.string());
  std::ifstream input_file(file_path);
  std::string line;
  while (std::getline(input_file, line)) {
    file_lines_.push_back(line);
  }

    mc_years_ = reader_.GetInteger("general", "nbyears", 0);
    user_playlist_ = reader_.GetBoolean("general", "user-playlist", false);
    playlist_reset_option_ = true;
}
std::vector<int> GeneralDataIniReader::GetActiveYears() {
  active_year_list_.clear();
  inactive_year_list_.clear();
  if (user_playlist_) {
    SetPlaylistResetOption();
    SetPlaylistYearLists();
    return GetActiveYears_();
  } else {
    std::vector<int> active_years(mc_years_);
    std::iota(std::begin(active_years), std::end(active_years), 1);
    return active_years;
  }
}

std::vector<int> GeneralDataIniReader::GetActiveYears_() {
  if (playlist_reset_option_) {
    return ActiveYearsFromInactiveList();
  } else {
    return ActiveYearsFromActiveList();
  }
}

std::vector<int> GeneralDataIniReader::ActiveYearsFromActiveList() {
  std::vector<int> active_years;
  for (auto year = 0; year < mc_years_; year++) {
    if (std::ranges::find(active_year_list_, year) !=
        active_year_list_.end()) {
      active_years.push_back(year + 1);
    }
  }
  return active_years;
}

std::vector<int> GeneralDataIniReader::ActiveYearsFromInactiveList() {
  std::vector<int> active_years;
  for (auto year = 0; year < mc_years_; year++) {
    if (std::ranges::find(inactive_year_list_,
                  year) == inactive_year_list_.end()) {
      active_years.push_back(year + 1);
    }
  }
  return active_years;
}

std::pair<std::vector<int>, std::vector<int>>
GeneralDataIniReader::GetRawPlaylist() {
  std::string current_section = "";
  active_year_list_.clear();
  inactive_year_list_.clear();
  for (const auto& line : file_lines_) {
    if (IniReaderUtils::LineIsNotASectionHeader(line)) {
      if (current_section == "playlist") {
        if (line.find("playlist_reset") != std::string::npos) {
          continue;
        }
        auto [key, val] = IniReaderUtils::GetKeyValFromLine(line);
        ReadPlaylistVal(key, val);
      }
    } else {
      current_section = IniReaderUtils::ReadSectionHeader(line);
    }
  }
  return {active_year_list_, inactive_year_list_};
}
void GeneralDataIniReader::SetPlaylistResetOption() {
  //  Default : all mc years are activated
  playlist_reset_option_ =
      reader_.GetBoolean("playlist", "playlist_reset", true);
}
void GeneralDataIniReader::SetPlaylistYearLists() {
  std::string current_section = "";
  for (const auto& line : file_lines_) {
    current_section = ReadPlaylist(current_section, StringManip::trim(line));
  }
}

std::string GeneralDataIniReader::ReadPlaylist(
    const std::string& current_section, const std::string& line) {
  if (IniReaderUtils::LineIsNotASectionHeader(line)) {
    if (current_section == "playlist") {
      ReadPlaylist(line);
    }
  } else {
    return IniReaderUtils::ReadSectionHeader(line);
  }
  return current_section;
}
void GeneralDataIniReader::ReadPlaylist(const std::string& line) {
  if (line.find("playlist_reset") == std::string::npos) {
    auto [key, val] = IniReaderUtils::GetKeyValFromLine(line);
    ReadPlaylistVal(key, val);
  }
}

void GeneralDataIniReader::ReadPlaylistVal(const std::string& key, int val) {
  if (key == "playlist_year +") {
    active_year_list_.push_back(val);
  } else if (key == "playlist_year -") {
    inactive_year_list_.push_back(val);
  }
}