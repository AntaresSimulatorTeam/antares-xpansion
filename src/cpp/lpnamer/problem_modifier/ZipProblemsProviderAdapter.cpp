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
    const std::string& solver_name) const {
  std::vector<std::shared_ptr<Problem>> problems;
  // Order is important. Problems need to be in the same order as names
  std::for_each(
      std::execution::seq /*keep it seq to avoid problems not being in the same
                             order as names*/
      ,
      problem_names_.begin(), problem_names_.end(), [&](auto name) {
        ZipProblemProviderAdapter problem_provider(lp_dir_, name,
                                                   archive_reader_);
        problems.push_back(problem_provider.provide_problem(solver_name));
      });

  return problems;
}
ZipProblemsProviderAdapter::ZipProblemsProviderAdapter(
    std::filesystem::path lp_dir, std::shared_ptr<ArchiveReader> archive_reader,
    std::vector<std::string> problem_names)
    : archive_reader_(std::move(archive_reader)),
      lp_dir_(std::move(lp_dir)),
      problem_names_(std::move(problem_names)) {}
