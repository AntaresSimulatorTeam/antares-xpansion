//
// Created by marechaljas on 25/11/22.
//

#pragma once

#include "antares-xpansion/archive_handler/ArchiveReader.h"
#include "IXpansionProblemsProvider.h"
class ZipProblemsProviderAdapter : public IXpansionProblemsProvider {
 public:
  ZipProblemsProviderAdapter(std::filesystem::path lp_dir,
                             std::shared_ptr<ArchiveReader> archive_reader,
                             std::vector<std::string> problem_names);
  [[nodiscard]] std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
  std::shared_ptr<ArchiveReader> archive_reader_;
  std::filesystem::path lp_dir_;
  std::vector<std::string> problem_names_;
};
