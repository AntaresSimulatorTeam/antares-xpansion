#include "glog/logging.h"
#include "helpers/Path.h"
#include "Worker.h"

#include "solver_utils.h"

/*!
 *  \brief Free the problem
 */
void Worker::free()
{
	if (_solver)
	{
		_solver.reset();
		_solver = nullptr;
	}
}

/*!
 *  \brief Return the optimal value of a problem
 *
 *  \param lb : double which receives the optimal value
 */
void Worker::get_value(double &lb) const
{
	if (_is_master && _solver->get_n_integer_vars() > 0)
	{
		lb = _solver->get_mip_value();
	}
	else
	{
		lb = _solver->get_lp_value();
	}
}

/*!
 *  \brief Initialization of a problem
 *
 *  \param variable_map : map linking each problem name to its variables and their ids
 *
 *  \param problem_name : name of the problem
 */
void Worker::init(Str2Int const &variable_map, std::string const &path_to_mps,
				  std::string const &solver_name, int log_level)
{
	_path_to_mps = path_to_mps;

	SolverFactory factory;
	if (_is_master)
	{
		_solver = factory.create_solver(solver_name, SOLVER_TYPE::INTEGER);
	}
	else
	{
		_solver = factory.create_solver(solver_name, SOLVER_TYPE::CONTINUOUS);
	}

	_solver->init();
	_solver->set_threads(1);
	_solver->set_output_log_level(log_level);
	_solver->read_prob_mps(path_to_mps);

	int var_index;
	for (auto const &kvp : variable_map)
	{
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
void Worker::solve(int &lp_status, const std::string &outputroot) const
{

	if (_is_master && _solver->get_n_integer_vars() > 0)
	{
		lp_status = _solver->solve_mip();
	}
	else
	{
		lp_status = _solver->solve_lp();
	}

	if (lp_status != SOLVER_STATUS::OPTIMAL)
	{
		LOG(INFO) << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;
		buffer << Path(outputroot) / (_path_to_mps + "_lp_status_") / (_solver->SOLVER_STRING_STATUS[lp_status] + MPS_SUFFIX);
		LOG(INFO) << "lp_status is : " << _solver->SOLVER_STRING_STATUS[lp_status] << std::endl;
		LOG(INFO) << "written in " << buffer.str() << std::endl;
		_solver->write_prob_mps(buffer.str());

		throw InvalidSolverStatusException("Invalid solver status " + _solver->SOLVER_STRING_STATUS[lp_status] + " optimality expected");
	}

	if (_is_master)
	{
		std::stringstream buffer;

		buffer << Path(outputroot) / insert_str_in_str(_path_to_mps, MPS_SUFFIX, "_last_iteration");
		_solver->write_prob_mps(buffer.str());
	}
}

std::string Worker::insert_str_in_str(const std::string &input, const std::string &suffix, const std::string &to_insert)
{
	return input.substr(0, input.length() - suffix.length()) + to_insert + suffix;
}
/*!
 *  \brief Get the number of iteration needed to solve a problem
 *
 *  \param result : result
 */
void Worker::get_splex_num_of_ite_last(int &result) const
{
	result = _solver->get_splex_num_of_ite_last();
}
