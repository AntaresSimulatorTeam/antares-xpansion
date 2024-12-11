//
// Created by marechaljas on 09/01/24.
//

#include <execution>
#include <tbb/tbb.h>

#include "antares-xpansion/lpnamer/problem_modifier/FileProblemsProviderAdapter.h"


#include "antares-xpansion/lpnamer/problem_modifier/FileProblemProviderAdapter.h"

std::vector<std::shared_ptr<Problem>>
FileProblemsProviderAdapter::provideProblems(
    const std::string& solver_name,
    SolverLogManager& solver_log_manager) const {
  std::vector<std::shared_ptr<Problem>> problems(problem_names_.size());
  // Order is important. Problems need to be in the same order as names
  std::transform(std::execution::par,
                 /*std::transform preserves order of element*/
                 problem_names_.begin(), problem_names_.end(), problems.begin(),
                 [&](auto name) {
                   FileProblemProviderAdapter problem_provider(lp_dir_, name);
                   return problem_provider.provide_problem(solver_name,
                                                           solver_log_manager);
                 });
  return problems;
}
FileProblemsProviderAdapter::FileProblemsProviderAdapter(
    std::filesystem::path lp_dir, std::vector<std::string> problem_names)
    : lp_dir_(lp_dir), problem_names_(problem_names) {}
