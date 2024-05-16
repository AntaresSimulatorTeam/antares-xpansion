#ifndef SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#define SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#include <filesystem>
#include <utility>

#include "LogUtils.h"
#include "Mode.h"
#include "ProblemGenerationLogger.h"

class LpFilesExtractor {
 private:
  const std::filesystem::path antares_archive_path_;
  const std::filesystem::path xpansion_output_dir_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  const Mode mode_;
  const std::filesystem::path& simulation_dir_;

 public:
  explicit LpFilesExtractor(
      std::filesystem::path antares_archive_path,
      std::filesystem::path xpansion_output_dir,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      Mode mode = Mode::UNKOWN,
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
  using areaAndIntecoPaths = std::pair<std::vector<std::filesystem::path>, std::vector<std::filesystem::path>>;
  [[nodiscard]] areaAndIntecoPaths getFiles() const;
  [[nodiscard]] areaAndIntecoPaths getFilesFromArchive() const;
  void checkProperNumberOfAreaFiles(
      const std::vector<std::filesystem::path>& vect_area_files) const;
  void produceAreatxtFile(const std::vector<std::filesystem::path>& vect_area_files) const;
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
