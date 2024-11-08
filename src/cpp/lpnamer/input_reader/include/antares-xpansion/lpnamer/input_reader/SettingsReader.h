#pragma once

#include <filesystem>
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

/**
 * A class to read the settings.ini file pertaining to antares-xpansion settings
 */
class SettingsReader {
 public:
  /**
  * Constructor for the SettingsReader class
  * @param file_path The path to the settings.ini file
   */
  explicit SettingsReader(const std::filesystem::path& file_path,
                          ProblemGenerationLog::ProblemGenerationLogger* logger);

  std::string Solver();

 private:
  ProblemGenerationLog::ProblemGenerationLogger* logger_;
  std::string solver_name_;

  std::map<std::string, std::string> settings_;
  void check_file_exist(const std::filesystem::path& file_path) const;
  void parse_file(const std::filesystem::path& file_path);
};