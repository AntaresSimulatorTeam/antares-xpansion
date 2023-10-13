//
// Created by marechaljas on 09/11/22.
//

#include "MPSFileProblemProviderAdapter.h"

#include <utility>

#include "multisolver_interface/SolverFactory.h"
std::shared_ptr<Problem> MPSFileProblemProviderAdapter::provide_problem(
    const std::string& solver_name,
    SolverLogManager& solver_log_manager) const {
  SolverFactory factory;
  auto const lp_mps_name = lp_dir_ / problem_name_;
  auto in_prblm = std::make_shared<Problem>(
      factory.create_solver(solver_name, solver_log_manager));

  in_prblm->read_prob_mps(lp_mps_name);
  return in_prblm;
}
MPSFileProblemProviderAdapter::MPSFileProblemProviderAdapter(
    std::filesystem::path root, const std::string& problem_name)
    : lp_dir_(std::move(root)), problem_name_(problem_name) {}
