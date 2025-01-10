
#include "antares-xpansion/multisolver_interface/SolverConfig.h"

#include <algorithm>
#include <utility>

const std::map<std::string, bool> SolverConfig::save_restore_support = {
    {"clp", false}, {"cbc", false}, {"coin", false}, {"xpress", true}};
SolverConfig::SolverConfig(std::string solver_name) {
  init(std::move(solver_name));
}
bool SolverConfig::operator==(const std::string& rhs) const {
  return std::ranges::equal(
      name, rhs, [](char a, char b) { return ::tolower(a) == ::tolower(b); });
}
SolverConfig& SolverConfig::operator=(const std::string& rhs) {
  init(rhs);
  return *this;
}
void SolverConfig::init(std::string solver_name) {
  std::ranges::transform(solver_name, std::back_inserter(name), ::tolower);
  save_restore_supported = save_restore_support.at(name);
}
