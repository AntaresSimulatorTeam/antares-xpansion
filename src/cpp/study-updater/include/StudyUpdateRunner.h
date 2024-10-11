#ifndef ANTARES_XPANSION_SRC_CPP_STUDY_UPDATER_STUDYUPDATERUNNER_H
#define ANTARES_XPANSION_SRC_CPP_STUDY_UPDATER_STUDYUPDATERUNNER_H
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "CandidatesINIReader.h"
#include "LauncherHelpers.h"
#include "LinkProfileReader.h"
#include "antares-xpansion/helpers/OptionsParser.h"
#include "StudyUpdater.h"
class StudyUpdaterExeOptions : public OptionsParser {
 private:
  std::filesystem::path xpansion_output_dir_;
  std::filesystem::path solution_file_;

 public:
  StudyUpdaterExeOptions();
  virtual ~StudyUpdaterExeOptions() = default;
  std::filesystem::path SolutionFile() const { return solution_file_; }
  std::filesystem::path XpansionOutputDir() const { return xpansion_output_dir_; }
};
void updateStudy(
    const std::filesystem::path &rootPath_p,
    const std::vector<ActiveLink> &links_p,
    const std::filesystem::path &jsonPath_l,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
#endif  // ANTARES_XPANSION_SRC_CPP_STUDY_UPDATER_STUDYUPDATERUNNER_H
