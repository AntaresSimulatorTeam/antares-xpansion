//
// Created by marechaljas on 09/01/24.
//

#include "FileProblemProviderAdapter.h"

#include <utility>

#include "multisolver_interface/SolverFactory.h"
std::shared_ptr<Problem> FileProblemProviderAdapter::provide_problem(
    const std::string& solver_name,
    SolverLogManager& solver_log_manager) const {
  SolverFactory factory;
  auto in_prblm = std::make_shared<Problem>(
      factory.create_solver(solver_name, solver_log_manager));

  in_prblm->read_prob_mps(lp_dir_.parent_path() / problem_name_);
  return in_prblm;
}
FileProblemProviderAdapter::FileProblemProviderAdapter(
    std::filesystem::path lp_dir, std::string problem_name)
    : lp_dir_(std::move(lp_dir)), problem_name_(std::move(problem_name)) {}
