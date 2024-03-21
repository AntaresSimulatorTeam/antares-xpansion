#pragma once
#include <regex>
#include <string>
#include <vector>

class VariablesGroup {
 public:
  explicit VariablesGroup(const std::vector<std::string>& all_variables,
                          const std::vector<std::regex>& patterns);
  std::vector<std::vector<int>> Indices() const { return indices_; }

 private:
  void Search();
  const std::vector<std::string>& all_variables_;
  std::vector<std::regex> patterns_;  // pos + zone1 // pos zon 2
  std::vector<std::vector<int>> indices_;
};