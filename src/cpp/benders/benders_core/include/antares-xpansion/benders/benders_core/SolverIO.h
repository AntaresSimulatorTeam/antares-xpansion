#pragma once

#include "ProblemFormat.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/multisolver_interface/SolverConfig.h"

class SolverIO {
  SolverConfig solver_config_{"Coin"};
  ProblemsFormat format_;
 public:
  void configure(const std::string& solver_name, ProblemsFormat format);
  void write(SolverAbstract* solver, const std::filesystem::path& path) const;
  void read(SolverAbstract* solver, const std::filesystem::path& path) const;
};
