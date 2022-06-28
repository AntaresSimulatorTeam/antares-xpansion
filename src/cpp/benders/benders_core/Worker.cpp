#include "Worker.h"

#include "glog/logging.h"
#include "solver_utils.h"

/*!
 *  \brief Free the problem
 */
void Worker::free() {
  if (solver_) {
    solver_.reset();
    solver_ = nullptr;
  }
}

/*!
 *  \brief Return the optimal value of a problem
 *
 *  \param lb : double which receives the optimal value
 */
void Worker::get_value(double &lb) const {
  if (_is_master && solver_->get_n_integer_vars() > 0) {
    lb = solver_->get_mip_value();
  } else {
    lb = solver_->get_lp_value();
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
void Worker::init(VariableMap const &variable_map,
                  const std::filesystem::path &path_to_mps,
                  std::string const &solver_name, int log_level,
                  const std::filesystem::path &log_name) {
  _path_to_mps = path_to_mps;

  SolverFactory factory;
  if (_is_master) {
    solver_ =
        factory.create_solver(solver_name, SOLVER_TYPE::INTEGER, log_name);
  } else {
    solver_ =
        factory.create_solver(solver_name, SOLVER_TYPE::CONTINUOUS, log_name);
  }

  solver_->init();
  solver_->set_threads(1);
  solver_->set_output_log_level(log_level);
  solver_->read_prob_mps(path_to_mps);

  int var_index;
  for (auto const &kvp : variable_map) {
    var_index = solver_->get_col_index(kvp.first);
    _id_to_name[var_index] = kvp.first;
    _name_to_id[kvp.first] = var_index;
  }
}

/*!
 *  \brief Method to solve a problem
 *
 *  \param lp_status : problem status after optimization
 */
void Worker::solve(int &lp_status, const std::string &outputroot,
                   const std::string &output_master_mps_file_name) const {
  if (_is_master && solver_->get_n_integer_vars() > 0) {
    lp_status = solver_->solve_mip();
  } else {
    lp_status = solver_->solve_lp();
  }

  if (lp_status != SOLVER_STATUS::OPTIMAL) {
    LOG(INFO) << "lp_status is : " << lp_status << std::endl;
    std::stringstream buffer;
    buffer << std::filesystem::path(outputroot) /
                  (_path_to_mps.string() + "_lp_status_") /
                  (solver_->SOLVER_STRING_STATUS[lp_status] + MPS_SUFFIX);
    LOG(INFO) << "lp_status is : " << solver_->SOLVER_STRING_STATUS[lp_status]
              << std::endl;
    LOG(INFO) << "written in " << buffer.str() << std::endl;
    solver_->write_prob_mps(buffer.str());

    throw InvalidSolverStatusException(
        "Invalid solver status " + solver_->SOLVER_STRING_STATUS[lp_status] +
        " optimality expected");
  }

  if (_is_master) {
    solver_->write_prob_mps(std::filesystem::path(outputroot) /
                            output_master_mps_file_name);
  }
}
/*!
 *  \brief Get the number of iteration needed to solve a problem
 *
 *  \param result : result
 */
void Worker::get_splex_num_of_ite_last(int &result) const {
  result = solver_->get_splex_num_of_ite_last();
}

void Worker::write_basis(const std::filesystem::path &filename) const {
  solver_->write_basis(filename);
}