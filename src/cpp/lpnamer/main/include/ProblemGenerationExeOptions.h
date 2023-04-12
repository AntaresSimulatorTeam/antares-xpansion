#ifndef ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
#define ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
#include <filesystem>
#include <vector>

#include "OptionsParser.h"

class ProblemGenerationExeOptions : public OptionsParser {
 private:
  std::filesystem::path xpansion_output_dir_;
  std::string master_formulation_;
  std::string additional_constraintFilename_l_;
  std::filesystem::path archive_path_;
  std::filesystem::path weights_file_ = "";
  std::vector<int> active_years_;
  bool rename_variables_ = false;

 public:
  ProblemGenerationExeOptions();

  virtual ~ProblemGenerationExeOptions() = default;
  std::filesystem::path XpansionOutputDir() const {
    return xpansion_output_dir_;
  }
  std::string MasterFormulation() const { return master_formulation_; }
  std::string AdditionalConstraintsFilename() const {
    return additional_constraintFilename_l_;
  }
  std::filesystem::path ArchivePath() const { return archive_path_; }
  std::filesystem::path WeightsFile() const { return weights_file_; }
  std::vector<int> ActiveYears() const { return active_years_; }
  bool RenameVariables() const { return rename_variables_; }
};
#endif  // ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
