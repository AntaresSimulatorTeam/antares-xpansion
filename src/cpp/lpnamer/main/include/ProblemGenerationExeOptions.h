#ifndef ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
#define ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
#include <filesystem>

#include "OptionsParser.h"
class ProblemGenerationExeOptions : public OptionsParser {
 private:
  std::filesystem::path root_;
  std::string master_formulation_;
  std::string additional_constraintFilename_l_;
  std::filesystem::path archive_path_;
  bool zip_mps_ = false;

 public:
  ProblemGenerationExeOptions();

  virtual ~ProblemGenerationExeOptions() = default;
  std::filesystem::path Root() const { return root_; }
  std::string MasterFormulation() const { return master_formulation_; }
  std::string AdditionalConstraintsFilename() const {
    return additional_constraintFilename_l_;
  }
  std::filesystem::path ArchivePath() const { return archive_path_; }
  bool ZipMps() const { return zip_mps_; }
};
#endif  // ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
