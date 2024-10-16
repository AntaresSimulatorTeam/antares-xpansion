//
// Created by marechaljas on 09/11/22.
//

#pragma once

#include "antares-xpansion/lpnamer/problem_modifier/IProblemProviderPort.h"
class MPSFileProblemProviderAdapter : public IProblemProviderPort {
 public:
  MPSFileProblemProviderAdapter(std::filesystem::path root,
                                const std::string& problem_name);
  [[nodiscard]] std::shared_ptr<Problem> provide_problem(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
  const std::filesystem::path lp_dir_;
  const std::string& problem_name_;
};
