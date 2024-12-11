//
// Created by marechaljas on 25/11/22.
//

#include <execution>
#include <tbb/tbb.h>

#include "antares-xpansion/lpnamer/problem_modifier/ZipProblemsProviderAdapter.h"

#include <utility>

#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/lpnamer/problem_modifier/ZipProblemProviderAdapter.h"

std::vector<std::shared_ptr<Problem>>
ZipProblemsProviderAdapter::provideProblems(
    const std::string& solver_name,
    SolverLogManager& solver_log_manager) const {
  std::vector<std::shared_ptr<Problem>> problems(problem_names_.size());
  // Order is important. Problems need to be in the same order as names
  std::transform(std::execution::par,
                 /*std::transform preserves order of element*/
                 problem_names_.begin(), problem_names_.end(), problems.begin(),
                 [&](auto name) {
                   ZipProblemProviderAdapter problem_provider(lp_dir_, name,
                                                              archive_reader_);
                   return problem_provider.provide_problem(solver_name,
                                                           solver_log_manager);
                 });
  return problems;
}
ZipProblemsProviderAdapter::ZipProblemsProviderAdapter(
    std::filesystem::path lp_dir, std::shared_ptr<ArchiveReader> archive_reader,
    std::vector<std::string> problem_names)
    : archive_reader_(std::move(archive_reader)),
      lp_dir_(std::move(lp_dir)),
      problem_names_(std::move(problem_names)) {}
