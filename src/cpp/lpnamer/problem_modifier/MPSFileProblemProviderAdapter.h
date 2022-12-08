//
// Created by marechaljas on 09/11/22.
//

#pragma once

#include "IProblemProviderPort.h"
class MPSFileProblemProviderAdapter : public IProblemProviderPort {
 public:
  MPSFileProblemProviderAdapter(const std::filesystem::path root,
                                const std::string& problem_name);
  [[nodiscard]] std::shared_ptr<Problem> provide_problem(
      const std::string& solver_name) const override;
  const std::filesystem::path lp_dir_;
  const std::string& problem_name_;
};
