//
// Created by marechaljas on 25/11/22.
//

#include "ZipProblemsProviderAdapter.h"

#include <execution>
#include <utility>

#include "ArchiveReader.h"
#include "ZipProblemProviderAdapter.h"

std::vector<std::shared_ptr<Problem>>
ZipProblemsProviderAdapter::provideProblems(
    const std::string& solver_name,
    const std::filesystem::path& log_file_path) const {
  std::vector<std::shared_ptr<Problem>> problems(problem_names_.size());
  // Order is important. Problems need to be in the same order as names
  std::transform(
      std::execution::par, /*std::transform preserves order of element*/
      problem_names_.begin(), problem_names_.end(), problems.begin(),
      [&](auto name) {
        ZipProblemProviderAdapter problem_provider(lp_dir_, name,
                                                             archive_reader_);
        std::cout << "Providing problem " << name << std::endl;
        return problem_provider.provide_problem(solver_name, log_file_path);
      });

  return problems;
}
ZipProblemsProviderAdapter::ZipProblemsProviderAdapter(
    std::filesystem::path lp_dir, std::shared_ptr<ArchiveReader> archive_reader,
    std::vector<std::string> problem_names)
    : archive_reader_(std::move(archive_reader)),
      lp_dir_(std::move(lp_dir)),
      problem_names_(std::move(problem_names)) {}
