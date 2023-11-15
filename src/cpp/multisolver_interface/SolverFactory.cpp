
#include "SolverXpress.h"
#include "multisolver_interface/environment.h"

#ifdef COIN_OR
#include "SolverCbc.h"
#include "SolverClp.h"
#endif
#include "LogUtils.h"
#include "multisolver_interface/SolverFactory.h"
std::vector<std::string> tmp;

std::vector<std::string> SolverLoader::GetAvailableSolvers() {
  if (tmp.empty()) {
    if (LoadXpress::XpressIsCorrectlyInstalled()) {
      tmp.push_back(XPRESS_STR);
    }
#ifdef COIN_OR
    tmp.push_back(CLP_STR);
    tmp.push_back(CBC_STR);
#endif
  }
  return tmp;
}

SolverFactory::SolverFactory()
    : _available_solvers(SolverLoader::GetAvailableSolvers()) {
  isXpress_available_ =
      std::find(tmp.cbegin(), tmp.cend(), XPRESS_STR) != tmp.cend();
}

SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name, const SOLVER_TYPE solver_type) const {
  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }

  else if (isXpress_available_ && solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>();
  }
#ifdef COIN_OR
  if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS) {
    return std::make_shared<SolverClp>();
  } else if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::INTEGER) {
    return std::make_shared<SolverCbc>();
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
}

SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name, const SOLVER_TYPE solver_type,
    SolverLogManager &log_manager) const {
#ifdef COIN_OR
  if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS) {
    return std::make_shared<SolverClp>(log_manager);
  } else if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::INTEGER) {
    return std::make_shared<SolverCbc>(log_manager);
  }
#endif
  return create_solver(solver_name, log_manager);
}

SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name) const {
  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  } else if (isXpress_available_ && solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>();
  }
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    return std::make_shared<SolverClp>();
  } else if (solver_name == CBC_STR) {
    return std::make_shared<SolverCbc>();
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
}

SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name, SolverLogManager &log_manager) const {
  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  if (isXpress_available_ && solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>(log_manager);
  }
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    return std::make_shared<SolverClp>(log_manager);
  } else if (solver_name == CBC_STR) {
    return std::make_shared<SolverCbc>(log_manager);
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
}

SolverAbstract::Ptr SolverFactory::copy_solver(
    const std::shared_ptr<const SolverAbstract> &to_copy) {
  std::string solver_name = to_copy->get_solver_name();

  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  if (isXpress_available_ && solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>(to_copy);
  }
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    return std::make_shared<SolverClp>(to_copy);
  } else if (solver_name == CBC_STR) {
    return std::make_shared<SolverCbc>(to_copy);
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
}

SolverAbstract::Ptr SolverFactory::copy_solver(SolverAbstract::Ptr to_copy) {
  return copy_solver(
      static_cast<const std::shared_ptr<const SolverAbstract>>(to_copy));
}

const std::vector<std::string> &SolverFactory::get_solvers_list() const {
  return _available_solvers;
}
