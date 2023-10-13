//
// Created by marechaljas on 03/11/22.
//

#pragma once

#include "../../lpnamer/model/Problem.h"

class IProblemProviderPort {
 public:
  virtual ~IProblemProviderPort() = default;
  [[nodiscard]] virtual std::shared_ptr<Problem> provide_problem(
      const std::string& solver_name,
      std::shared_ptr<SolverLogManager>& solver_log_manager) const = 0;
};
