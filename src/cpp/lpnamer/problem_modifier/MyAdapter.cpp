//
// Created by marechaljas on 02/11/22.
//

#include <algorithm>
#include <utility>

#include "LinkProblemsGenerator.h"
#include "VariableFileReader.h"
#include "ZipProblemProviderAdapter.h"
#include "helpers/StringUtils.h"
#include "solver_utils.h"
void ZipProblemProviderAdapter::reader_extract_file(
    const std::string& problem_name, ArchiveReader& reader,
    std::filesystem::path lpDir) const {
  reader.ExtractFile(problem_name, lpDir);
}

std::shared_ptr<Problem> ZipProblemProviderAdapter::provide_problem(
    const std::string& solver_name) const {
  reader_extract_file(problem_name, *archive_reader_, lp_dir_);
  SolverFactory factory;
  auto const lp_mps_name = ZipProblemProviderAdapter::lp_dir_ /
                           ZipProblemProviderAdapter::problem_name;
  auto in_prblm = std::make_shared<Problem>(factory.create_solver(solver_name));

  in_prblm->read_prob_mps(lp_mps_name);
  return in_prblm;
}

ZipProblemProviderAdapter::ZipProblemProviderAdapter(
    std::filesystem::path lp_dir, const std::string& data,
    std::shared_ptr<ArchiveReader> ptr)
    : lp_dir_(std::move(lp_dir)),
      problem_name(std::move(data)),
      archive_reader_(std::move(ptr)) {}
