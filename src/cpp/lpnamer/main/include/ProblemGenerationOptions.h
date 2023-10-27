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
};
