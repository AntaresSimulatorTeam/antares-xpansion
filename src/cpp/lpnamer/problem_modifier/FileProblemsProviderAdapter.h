//
// Created by marechaljas on 09/01/24.
//

#pragma once

#include "IXpansionProblemsProvider.h"

class FileProblemsProviderAdapter : public IXpansionProblemsProvider {
 public:
  FileProblemsProviderAdapter(std::filesystem::path lp_dir,
                              std::vector<std::string> problem_names);
  ~FileProblemsProviderAdapter() = default;
  std::vector<std::shared_ptr<Problem>> provideProblems(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
  std::filesystem::path lp_dir_;
  std::vector<std::string> problem_names_;
};
