#ifdef CPLEX
#include "SolverCplex.h"
#endif
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
#ifdef CPLEX
  _available_solvers.push_back(CPLEX_STR);
#endif
#ifdef XPRESS
  _available_solvers.push_back(XPRESS_STR);
#endif
#ifdef COIN_OR
  _available_solvers.push_back(CLP_STR);
  _available_solvers.push_back(CBC_STR);
#endif
}

SolverAbstract::Ptr
SolverFactory::create_solver(const std::string &solver_name,
                             const SOLVER_TYPE solver_type) const {
  return create_solver(solver_name, solver_type, NULL);
}
SolverAbstract::Ptr SolverFactory::create_solver(const std::string &solver_name,
                                                 const SOLVER_TYPE solver_type,
                                                 FILE *fp) const {

#ifdef COIN_OR
  if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::CONTINUOUS) {
    return std::make_shared<SolverClp>(fp);
  } else if (solver_name == COIN_STR && solver_type == SOLVER_TYPE::INTEGER) {
    return std::make_shared<SolverCbc>(fp);
  }
#endif
  return create_solver(solver_name, fp);
}
SolverAbstract::Ptr
SolverFactory::create_solver(const std::string &solver_name) const {
  return create_solver(solver_name, NULL);
}
SolverAbstract::Ptr SolverFactory::create_solver(const std::string &solver_name,
                                                 FILE *fp) const {

  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name);
  }
#ifdef CPLEX
  else if (solver_name == CPLEX_STR) {
    return std::make_shared<SolverCplex>();
  }
#endif
#ifdef XPRESS
  else if (solver_name == XPRESS_STR) {
    return std::make_shared<SolverXpress>(fp);
  }
#endif
#ifdef COIN_OR
  else if (solver_name == CLP_STR) {
    return std::make_shared<SolverClp>(fp);
  } else if (solver_name == CBC_STR) {
    return std::make_shared<SolverCbc>(fp);
  }
#endif
  else {
    throw InvalidSolverNameException(solver_name);
  }
}

SolverAbstract::Ptr SolverFactory::create_solver(SolverAbstract::Ptr to_copy) {
  std::string solver_name = to_copy->get_solver_name();

  if (solver_name == "") {
    throw InvalidSolverNameException(solver_name);
  }
#ifdef CPLEX
  else if (solver_name == CPLEX_STR) {
    return std::make_shared<SolverCplex>(to_copy);
  }
#endif
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

const std::vector<std::string> &SolverFactory::get_solvers_list() const {
  return _available_solvers;
}
