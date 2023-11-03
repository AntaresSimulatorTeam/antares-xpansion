//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <filesystem>
#include <string>

#include "ProblemGenerationExeOptions.h"
#include "ProblemGenerationLogger.h"
#include "ProblemGenerationOptions.h"

class ProblemGeneration {
 public:
  explicit ProblemGeneration(ProblemGenerationOptions& options);
  std::filesystem::path updateProblems();
  ProblemGenerationOptions& options_;

  virtual void RunProblemGeneration(
      const std::filesystem::path& xpansion_output_dir,
      const std::string& master_formulation,
      const std::string& additionalConstraintFilename_l,
      const std::filesystem::path& archive_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      const std::filesystem::path& log_file_path,
      const std::filesystem::path& weights_file, bool unnamed_problems);

  class MismatchedParameters
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;

   public:
    explicit MismatchedParameters(const std::string& err_message,
                                  const std::string& log_location);
  };

 private:
  void ProcessWeights(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& antares_archive_path,
      const std::filesystem::path& weights_file,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  void ExtractUtilsFiles(
      const std::filesystem::path& antares_archive_path,
      const std::filesystem::path& xpansion_output_dir,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  static std::filesystem::path deduceXpansionDirIfEmpty(
      std::filesystem::path xpansion_output_dir,
      const std::filesystem::path& archive_path);
  static std::filesystem::path deduceArchivePathIfEmpty(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& archive_path);
};
