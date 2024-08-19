#ifndef SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H
#define SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "INIReader.h"
#include "ProblemGenerationLogger.h"
#include "StringManip.h"

class IniFileNotFound : public std::runtime_error {
 public:
  explicit IniFileNotFound(const std::string& msg) : std::runtime_error(msg) {}
};

class GeneralDataIniReader {
 private:
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  INIReader reader_;
  int mc_years_;
  bool user_playlist_;
  bool playlist_reset_option_;
  std::vector<int> active_year_list_;
  std::vector<int> inactive_year_list_;
  std::vector<std::string> file_lines_;

  std::vector<int> GetActiveYears_();
  std::vector<int> ActiveYearsFromActiveList();
  std::vector<int> ActiveYearsFromInactiveList();
  void SetPlaylistResetOption();
  void SetPlaylistYearLists();
  std::string ReadPlaylist(const std::string& current_section,
                           const std::string& line);
  void ReadPlaylistVal(const std::string& key, int val);
  void ReadPlaylist(const std::string& line);

 public:
  explicit GeneralDataIniReader(
      const std::filesystem::path& file_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

  int GetNbYears() const { return mc_years_; }

  std::vector<int> GetActiveYears();
  std::pair<std::vector<int>, std::vector<int>> GetRawPlaylist();
};

#endif  // SRC_CPP_LPNAMER_INPUTREADER_GENERALDATAREADER_H
