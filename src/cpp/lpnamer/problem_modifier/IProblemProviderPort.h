//
// Created by marechaljas on 03/11/22.
//

#pragma once

#include <cstdio>

#include "../../lpnamer/model/Problem.h"

class IProblemProviderPort {
 public:
  virtual ~IProblemProviderPort() = default;
  [[nodiscard]] virtual std::shared_ptr<Problem> provide_problem(
      const std::string& solver_name, FILE* log_file_ptr) const = 0;
};
