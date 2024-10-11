#ifndef SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#define SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#include <filesystem>
#include <utility>

#include "LogUtils.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/model/SimulationInputMode.h"

class LpFilesExtractor {
 private:
  const std::filesystem::path antares_archive_path_;
  const std::filesystem::path xpansion_output_dir_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  const SimulationInputMode mode_;
  const std::filesystem::path& simulation_dir_;

 public:
  explicit LpFilesExtractor(
      std::filesystem::path antares_archive_path,
      std::filesystem::path xpansion_output_dir,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      SimulationInputMode mode,
      const std::filesystem::path& simulation_dir = {})
      : antares_archive_path_(std::move(antares_archive_path)),
        xpansion_output_dir_(std::move(xpansion_output_dir)),
        logger_(std::move(logger)),
        mode_(mode),
        simulation_dir_(simulation_dir) {}
  void ExtractFiles() const;

  class ErrorWithAreaFile : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };
  class ErrorWithIntercosFile : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };
 private:
  using areaAndIntercoPaths = std::pair<std::vector<std::filesystem::path>, std::vector<std::filesystem::path>>;
  [[nodiscard]] areaAndIntercoPaths getFiles() const;
  [[nodiscard]] areaAndIntercoPaths getFilesFromArchive() const;
  void checkProperNumberOfAreaFiles(
      const std::vector<std::filesystem::path>& vect_area_files) const;
  void produceAreatxtFile(const std::vector<std::filesystem::path>& vect_area_files) const;
  void checkProperNumberOfIntercoFiles(
      const std::vector<std::filesystem::path>& vect_interco_files) const;
  void produceIntercotxtFile(
      const std::vector<std::filesystem::path>& vect_interco_files) const;
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
