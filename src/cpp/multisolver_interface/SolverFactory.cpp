#ifdef XPRESS
#include "SolverXpress.h"
#endif
#ifdef COIN_OR
#include "SolverCbc.h"
#include "SolverClp.h"
#endif

#include "multisolver_interface/SolverFactory.h"

SolverFactory::SolverFactory() {
  _available_solvers.clear();
#ifdef XPRESS
  _available_solvers.push_back(XPRESS_STR);
#endif
#ifdef COIN_OR
  _available_solvers.push_back(CLP_STR);
  _available_solvers.push_back(CBC_STR);
#endif
}

SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name, const SOLVER_TYPE solver_type) const {
  return create_solver(solver_name, solver_type, "");
}
SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name, const SOLVER_TYPE solver_type,
    const std::filesystem::path &log_name) const {
#ifdef COIN_OR
  if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS) {
    return std::make_shared<SolverClp>(log_name);
  } else if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::INTEGER) {
    return std::make_shared<SolverCbc>(log_name);
  }
#endif
  return create_solver(solver_name, log_name);
}
SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name) const {
  return create_solver(solver_name, "");
}
SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name,
    const std::filesystem::path &log_name) const {
  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name);
  }
#ifdef XPRESS
  else if (solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>(log_name);
  }
#endif
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    return std::make_shared<SolverClp>(log_name);
  } else if (solver_name == CBC_STR) {
    return std::make_shared<SolverCbc>(log_name);
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name);
  }
}

SolverAbstract::Ptr SolverFactory::copy_solver(
    const std::shared_ptr<const SolverAbstract> &to_copy) {
  std::string solver_name = to_copy->get_solver_name();

  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name);
  }
#ifdef XPRESS
  else if (solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>(to_copy);
  }
#endif
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    return std::make_shared<SolverClp>(to_copy);
  } else if (solver_name == CBC_STR) {
    return std::make_shared<SolverCbc>(to_copy);
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name);
  }
}

SolverAbstract::Ptr SolverFactory::copy_solver(SolverAbstract::Ptr to_copy) {
  return copy_solver(
      static_cast<const std::shared_ptr<const SolverAbstract>>(to_copy));
}

const std::vector<std::string> &SolverFactory::get_solvers_list() const {
  return _available_solvers;
}
