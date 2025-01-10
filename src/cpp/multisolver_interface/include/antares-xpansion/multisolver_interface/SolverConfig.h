#pragma once

#include <map>
#include <string>

/**
 * @brief Class to store the configuration of a solver
 * Invariant: name is lowercase
 */
class SolverConfig {
  static const std::map<std::string, bool> save_restore_support;
  void init(std::string solver_name);

 public:
  explicit SolverConfig(std::string name);
  SolverConfig(SolverConfig&&) = default;
  SolverConfig(const SolverConfig&) = default;
  SolverConfig& operator=(const SolverConfig&) = default;
  SolverConfig& operator=(SolverConfig&&) = default;
  ~SolverConfig() = default;

  std::string name;
  bool save_restore_supported{false};
  bool operator==(const std::string& rhs) const;
  SolverConfig& operator=(const std::string& rhs);
};
