#include "Worker.h"

#include "LogUtils.h"

#include "solver_utils.h"
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
void Worker::get_value(double &lb) const {
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
void Worker::init(VariableMap const &variable_map,
                  const std::filesystem::path &path_to_mps,
                  std::string const &solver_name, int log_level,
                  SolverLogManager&solver_log_manager) {
  _path_to_mps = path_to_mps;
  SolverFactory factory(logger_);

  if (_is_master) {
    _solver = factory.create_solver(solver_name, SOLVER_TYPE::INTEGER,
                                    solver_log_manager);
  } else {
    _solver = factory.create_solver(solver_name, SOLVER_TYPE::CONTINUOUS,
                                    solver_log_manager);
  }

  _solver->set_output_log_level(log_level);
  _solver->read_prob_mps(path_to_mps, true);
  _solver->set_threads(1);

  _name_to_id = variable_map;

  for (auto const &kvp : variable_map) {
    _id_to_name[kvp.second] = kvp.first;
  }
}

/*!
 *  \brief Method to solve a problem
 *
 *  \param lp_status : problem status after optimization
 */
void Worker::solve(int &lp_status, const std::string &outputroot,
                   const std::string &output_master_mps_file_name,
                   Writer writer) const {
  if (_is_master && _solver->get_n_integer_vars() > 0) {
    lp_status = _solver->solve_mip();
  } else {
    lp_status = _solver->solve_lp();
  }

  if (lp_status != SOLVER_STATUS::OPTIMAL) {
    std::filesystem::path error_file_path;
    auto problem_status = _solver->SOLVER_STRING_STATUS[lp_status];
    error_file_path = std::filesystem::path(outputroot) /
                      (_path_to_mps.filename().string() + "_lp_status_" +
                       problem_status + MPS_SUFFIX);
    std::ostringstream msg;
    msg << "lp_status is : " << problem_status << std::endl;

    msg << "written in " << error_file_path.string() << std::endl;
    logger_->display_message(msg.str());
    _solver->write_prob_mps(error_file_path);
    Output::ProblemData data;
    data.name = _path_to_mps.filename().string();
    data.path = error_file_path;
    data.status = problem_status;
    writer->WriteProblem(data);
    writer->dump();
    auto log_location = LOGLOCATION;
    msg.str("");
    msg << "Invalid solver status " + problem_status + " optimality expected";
    logger_->display_message(log_location + msg.str());
    throw InvalidSolverStatusException(msg.str(), log_location);
  }

  if (_is_master) {
    _solver->write_prob_mps(std::filesystem::path(outputroot) /
                            output_master_mps_file_name);
  }
}
/*!
 *  \brief Get the number of iteration needed to solve a problem
 *
 *  \param result : result
 */
void Worker::get_splex_num_of_ite_last(int &result) const {
  result = _solver->get_splex_num_of_ite_last();
}

void Worker::write_basis(const std::filesystem::path &filename) const {
  _solver->write_basis(filename);
}

int Worker::RowIndex(const std::string &row_name) const {
  return _solver->get_row_index(row_name);
}

void Worker::ChangeRhs(int id_row, double val) const {
  _solver->chg_rhs(id_row, val);
}

void Worker::GetRhs(double *val, int id_row) const {
  _solver->get_rhs(val, id_row, id_row);
}

void Worker::AddRows(std::vector<char> const &qrtype_p,
                     std::vector<double> const &rhs_p,
                     std::vector<double> const &range_p,
                     std::vector<int> const &mstart_p,
                     std::vector<int> const &mclind_p,
                     std::vector<double> const &dmatval_p,
                     const std::vector<std::string> &row_names) const {
  solver_addrows(*_solver, qrtype_p, rhs_p, {}, mstart_p, mclind_p, dmatval_p,
                 row_names);
}

int Worker::Getnrows() const { return _solver->get_nrows(); }
int Worker::Getncols() const { return _solver->get_ncols(); }
