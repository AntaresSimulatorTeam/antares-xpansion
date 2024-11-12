
#include "SolverXpress.h"
#include "antares-xpansion/multisolver_interface/environment.h"

#ifdef COIN_OR
#include "SolverCbc.h"
#include "SolverClp.h"
#endif
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
std::vector<std::string> available_solvers;

std::vector<std::string> SolverLoader::GetAvailableSolvers(
    std::shared_ptr<ILoggerXpansion> logger) {
  if (available_solvers.empty()) {
    LoadXpress::XpressLoader xpress_loader(logger);
    if (xpress_loader.XpressIsCorrectlyInstalled(true)) {
      available_solvers.push_back(XPRESS_STR);
    }
#ifdef COIN_OR
    available_solvers.push_back(CLP_STR);
    available_solvers.push_back(CBC_STR);
#endif
  }
  return available_solvers;
}

SolverFactory::SolverFactory(std::shared_ptr<ILoggerXpansion> logger)
    : _available_solvers(SolverLoader::GetAvailableSolvers(logger)),
      logger_(std::move(logger)) {
  isXpress_available_ =
      std::find(available_solvers.cbegin(), available_solvers.cend(),
                XPRESS_STR) != available_solvers.cend();
}

SolverAbstract::Ptr SolverFactory::create_solver(
    const std::string &solver_name, const SOLVER_TYPE solver_type) const {
  SolverAbstract::Ptr ret;
  if (solver_name.empty()) {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  else if (isXpress_available_ && solver_name == XPRESS_STR) {
    ret = std::make_shared<SolverXpress>();
  }
#ifdef COIN_OR
  if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS) {
    ret = std::make_shared<SolverClp>();
  } else if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::INTEGER) {
    ret = std::make_shared<SolverCbc>();
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  ret->init();
  return ret;
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
    std::string solver_name) const {
  SolverAbstract::Ptr ret;
  std::transform(solver_name.begin(), solver_name.end(), solver_name.begin(), ::toupper);
  if (solver_name.empty()) {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  } else if (isXpress_available_ && solver_name == XPRESS_STR) {
    ret = std::make_shared<SolverXpress>();
  }
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    ret = std::make_shared<SolverClp>();
  } else if (solver_name == CBC_STR) {
    ret = std::make_shared<SolverCbc>();
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  ret->init();
  return ret;
}

SolverAbstract::Ptr SolverFactory::create_solver(
    std::string solver_name, SolverLogManager &log_manager) const {
  if (solver_name.empty()) {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  std::transform(solver_name.begin(), solver_name.end(), solver_name.begin(), ::toupper);
  SolverAbstract::Ptr ret;
  if (isXpress_available_ && solver_name == XPRESS_STR) {
    ret = std::make_shared<SolverXpress>(log_manager);
  }
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    ret = std::make_shared<SolverClp>(log_manager);
  } else if (solver_name == CBC_STR) {
    ret = std::make_shared<SolverCbc>(log_manager);
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name, LOGLOCATION);
  }
  ret->init();
  return ret;
}

SolverAbstract::Ptr SolverFactory::copy_solver(
    const std::shared_ptr<const SolverAbstract> &to_copy) const {
  std::string solver_name = to_copy->get_solver_name();
  std::transform(solver_name.begin(), solver_name.end(), solver_name.begin(), ::toupper);
  if (solver_name.empty()) {
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
