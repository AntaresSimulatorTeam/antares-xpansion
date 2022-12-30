#ifndef SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#define SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#include <filesystem>
#include <stdexcept>

#include "ProblemGenerationLogger.h"

class LpFilesExtractor {
 private:
  std::filesystem::path antares_archive_path_;
  std::filesystem::path xpansion_output_dir_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;

 public:
  explicit LpFilesExtractor(
      const std::filesystem::path& antares_archive_path,
      const std::filesystem::path& xpansion_output_dir,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : antares_archive_path_(antares_archive_path),
        xpansion_output_dir_(xpansion_output_dir),
        logger_(logger) {}
  void ExtractFiles() const;

  class ErrorWithAreaFile : public std::runtime_error {
   public:
    ErrorWithAreaFile(const std::string& err_message)
        : std::runtime_error(err_message) {}
  };
  class ErrorWithIntercosFile : public std::runtime_error {
   public:
    ErrorWithIntercosFile(const std::string& err_message)
        : std::runtime_error(err_message) {}
  };
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
