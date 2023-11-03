//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <filesystem>
#include <vector>

class ProblemGenerationOptions {
 public:
  virtual ~ProblemGenerationOptions() = default;
  [[nodiscard]] virtual std::filesystem::path XpansionOutputDir() const = 0;
  [[nodiscard]] virtual std::string MasterFormulation() const = 0;
  [[nodiscard]] virtual std::string AdditionalConstraintsFilename() const = 0;
  [[nodiscard]] virtual std::filesystem::path ArchivePath() const = 0;
  [[nodiscard]] virtual std::filesystem::path WeightsFile() const = 0;
  [[nodiscard]] virtual std::vector<int> ActiveYears() const = 0;
  [[nodiscard]] virtual bool UnnamedProblems() const = 0;
  [[nodiscard]] virtual std::filesystem::path deduceXpansionDirIfEmpty(
      std::filesystem::path xpansion_output_dir,
      const std::filesystem::path& archive_path) const = 0;
  [[nodiscard]] virtual std::filesystem::path deduceArchivePathIfEmpty(
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& archive_path) const = 0;

  class MismatchedParameters
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;

   public:
    explicit MismatchedParameters(const std::string& err_message,
                                  const std::string& log_location)
        : XpansionError(err_message, log_location) {}
  };

  class MissingParameters : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;

   public:
    explicit MissingParameters(const std::string& err_message,
                               const std::string& log_location)
        : XpansionError(err_message, log_location) {}
  };
};