#include "Worker.h"
#include "glog/logging.h"
#include "helpers/Path.h"

#include "solver_utils.h"

Worker::Worker() : _is_master(false), _solver(nullptr) {}

Worker::~Worker() {}

/*!
 *  \brief Free the problem
 */
void Worker::free() {
  if (_solver) {
    _solver.reset();
    _solver = nullptr;
  }
}

/*!
 *  \brief Return the optimal value of a problem
 *
 *  \param lb : double which receives the optimal value
 */
void Worker::get_value(double &lb) {
  if (_is_master && _solver->get_n_integer_vars() > 0) {
    lb = _solver->get_mip_value();
  } else {
    lb = _solver->get_lp_value();
  }
}

/*!
 *  \brief Initialization of a problem
 *
 *  \param variable_map : map linking each problem name to its variables and
 * their ids
 *
 *  \param problem_name : name of the problem
 */
void Worker::init(Str2Int const &variable_map, std::string const &path_to_mps,
                  std::string const &solver_name, int log_level) {
  _path_to_mps = path_to_mps;

  SolverFactory factory;
  if (_is_master) {
    if (solver_name == "COIN") {
      _solver = factory.create_solver("CBC");
    } else {
      _solver = factory.create_solver(solver_name);
    }
    _solver->init();

  } else {
    if (solver_name == "COIN") {
      _solver = factory.create_solver("CLP");
    } else {
      _solver = factory.create_solver(solver_name);
    }
    _solver->init();
  }
  _solver->set_threads(1);
  _solver->set_output_log_level(log_level);
  _solver->read_prob_mps(path_to_mps);

  int var_index;
  for (auto const &kvp : variable_map) {
    var_index = _solver->get_col_index(kvp.first);
    _id_to_name[var_index] = kvp.first;
    _name_to_id[kvp.first] = var_index;
  }
}

/*!
 *  \brief Method to solve a problem
 *
 *  \param lp_status : problem status after optimization
 */
void Worker::solve(int &lp_status, BendersOptions const &options) {

  if (_is_master && _solver->get_n_integer_vars() > 0) {
    lp_status = _solver->solve_mip();
  } else {
    lp_status = _solver->solve_lp();
  }

  if (lp_status != SOLVER_STATUS::OPTIMAL) {
    LOG(INFO) << "lp_status is : " << lp_status << std::endl;
    std::stringstream buffer;
    buffer << Path(options.OUTPUTROOT) / (_path_to_mps + "_lp_status_") /
                  (_solver->SOLVER_STRING_STATUS[lp_status] + ".mps");
    LOG(INFO) << "lp_status is : " << _solver->SOLVER_STRING_STATUS[lp_status]
              << std::endl;
    LOG(INFO) << "written in " << buffer.str() << std::endl;
    _solver->write_prob_mps(buffer.str());

    throw InvalidSolverStatusException(
        "Invalid solver status " + _solver->SOLVER_STRING_STATUS[lp_status] +
        " optimality expected");
  }

  if (_is_master) {
    std::string suffix = ".mps";
    std::stringstream buffer;
    buffer << Path(options.OUTPUTROOT) /
                  _path_to_mps.substr(0,
                                      _path_to_mps.length() - suffix.length())
           << "_last_iteration";
    buffer << ".mps";
    _solver->write_prob_mps(buffer.str());
  }
}

/*!
 *  \brief Get the number of iteration needed to solve a problem
 *
 *  \param result : result
 */
void Worker::get_simplex_ite(int &result) {
  result = _solver->get_simplex_ite();
}
