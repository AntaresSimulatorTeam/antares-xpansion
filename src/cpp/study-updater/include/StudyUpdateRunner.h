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
#include "OptionsParser.h"
#include "StudyUpdater.h"
class StudyUpdaterExeOptions : public OptionsParser {
 private:
  std::filesystem::path root_;
  std::filesystem::path solution_file_;

 public:
  StudyUpdaterExeOptions() : OptionsParser("Study-updater exe"){};
  virtual ~StudyUpdaterExeOptions() = default;
  std::filesystem::path SolutionFile() const { return solution_file_; }
  std::filesystem::path Root() const { return root_; }
};
void updateStudy(const std::filesystem::path &rootPath_p,
                 const std::vector<ActiveLink> &links_p,
                 const std::filesystem::path &jsonPath_l);
#endif  // ANTARES_XPANSION_SRC_CPP_STUDY_UPDATER_STUDYUPDATERUNNER_H
