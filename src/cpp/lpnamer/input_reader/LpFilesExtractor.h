#ifndef SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#define SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
#include <filesystem>

#include "LogUtils.h"
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

  class ErrorWithAreaFile : public XpansionError<std::runtime_error> {
   public:
    explicit ErrorWithAreaFile(const std::string& err_message,
                               const std::string& log_location)
        : XpansionError(err_message, log_location) {}
  };
  class ErrorWithIntercosFile : public XpansionError<std::runtime_error> {
   public:
    explicit ErrorWithIntercosFile(const std::string& err_message,
                                   const std::string& log_location)
        : XpansionError(err_message, log_location) {}
  };
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_LP_FILES_EXTRACTOR_H
