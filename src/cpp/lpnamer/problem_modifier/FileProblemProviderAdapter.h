//
// Created by marechaljas on 09/01/24.
//

#pragma once

#include "IProblemProviderPort.h"
class FileProblemProviderAdapter : public IProblemProviderPort {
 public:
  explicit FileProblemProviderAdapter(std::filesystem::path lp_dir,
                                      std::string problem_name);
  virtual ~FileProblemProviderAdapter() = default;
  std::shared_ptr<Problem> provide_problem(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
  std::filesystem::path lp_dir_;
  std::string problem_name_;
};
