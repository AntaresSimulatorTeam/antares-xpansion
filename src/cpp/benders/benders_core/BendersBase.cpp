#include "BendersBase.h"
#include "Timer.h"
#include "launcher.h"
#include "solver_utils.h"
#include "helpers/Path.h"

#include "glog/logging.h"

BendersBase::BendersBase(BendersBaseOptions const &options, Logger &logger, Writer writer) : _options(options), _logger(logger), _writer(writer) {}

/*!
 *  \brief Initialize set of data used in the loop
 */
void BendersBase::init_data()
{
	_data.nbasis = 0;
	_data.lb = -1e20;
	_data.ub = +1e20;
	_data.best_ub = +1e20;
	_data.stop = false;
	_data.it = 0;
	_data.alpha = 0;
	_data.invest_cost = 0;
	_data.deletedcut = 0;
	_data.maxsimplexiter = 0;
	_data.minsimplexiter = std::numeric_limits<int>::max();
	_data.best_it = 0;
	_data.stopping_criterion = StoppingCriterion::empty;
}

/*!
 *  \brief Print the trace of the Benders algorithm in a csv file
 *
 *  Method to print trace of the Benders algorithm in a csv file
 *
 */
void BendersBase::print_csv()
{
	std::string const output(Path(_options.OUTPUTROOT) / (_options.CSV_NAME + ".csv"));
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file)
	{
		file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;alpha_i;deletedcut;time;basis;" << std::endl;
		int const nite = _trace.size();
		for (int i = 0; i < nite; i++)
		{
			print_csv_iteration(file, i);
		}
		file.close();
	}
	else
	{
		LOG(INFO) << "Impossible to open the .csv file" << std::endl;
	}
}

void BendersBase::print_csv_iteration(std::ostream &file, int ite)
{

	if (_trace[ite]->_valid)
	{
		Point xopt;
		// Write first problem : use result of best iteration
		if (ite == 0)
		{
			int best_it_index = _data.best_it - 1;
			if (best_it_index >= 0 && _trace.size() > best_it_index)
			{
				xopt = _trace[best_it_index]->get_point();
			}
		}
		else
		{
			xopt = _trace[ite - 1]->get_point();
		}
		print_master_and_cut(file, ite + 1, _trace[ite], xopt);
	}
}
void BendersBase::print_master_and_cut(std::ostream &file, int ite, WorkerMasterDataPtr &trace, Point const &xopt)
{
	file << ite << ";";

	print_master_csv(file, trace, xopt);

	for (auto &kvp : trace->_cut_trace)
	{
		const SlaveCutDataHandler handler(kvp.second);
		file << ite << ";";
		print_cut_csv(file, handler, kvp.first, _problem_to_id[kvp.first]);
	}
}

/*!
 *  \brief Print in a file master's information
 *
 *  \param stream : output stream
 *
 *  \param trace : storage of problem data
 *
 *  \param xopt : final optimal value
 *
 *  \param name : master name
 *
 *  \param nslaves : number of slaves
 */
void BendersBase::print_master_csv(std::ostream &stream, const WorkerMasterDataPtr &trace, Point const &xopt) const
{
	stream << "Master"
		   << ";";
	stream << _options.MASTER_NAME << ";";
	stream << _data.nslaves << ";";
	stream << trace->_ub << ";";
	stream << trace->_lb << ";";
	stream << trace->_bestub << ";";
	stream << ";";
	stream << norm_point(xopt, trace->get_point()) << ";";
	stream << ";";
	stream << trace->_deleted_cut << ";";
	stream << trace->_time << ";";
	stream << trace->_nbasis << ";" << std::endl;
}

/*!
 *  \brief Print in a file slave's information
 *
 *  \param stream : output stream
 *
 *  \param handler : handler to manage slave data
 *
 *  \param name : problem name
 *
 *  \param islaves : problem id
 */
void BendersBase::print_cut_csv(std::ostream &stream, SlaveCutDataHandler const &handler, std::string const &name, int const islaves) const
{
	stream << "Slave"
		   << ";";
	stream << name << ";";
	stream << islaves << ";";
	stream << handler.get_dbl(SLAVE_COST) << ";";
	stream << ";";
	stream << ";";
	stream << handler.get_int(SIMPLEXITER) << ";";
	stream << ";";
	stream << handler.get_dbl(ALPHA_I) << ";";
	stream << ";";
	stream << handler.get_dbl(SLAVE_TIMER) << ";";
	stream << std::endl;
}

/*!
 *  \brief Update best upper bound and best optimal variables
 *
 *	Function to update best upper bound and best optimal variables regarding the current ones
 *
 *  \param best_ub : current best upper bound
 *
 *  \param ub : current upper bound
 *
 *  \param bestx : current best optimal variables
 *
 *  \param x0 : current optimal variables
 */
void BendersBase::update_best_ub()
{
	if (_data.best_ub > _data.ub)
	{
		_data.best_ub = _data.ub;
		_data.bestx = _data.x0;
		_data.best_it = _data.it;
	}
}

/*!
 *	\brief Update maximum and minimum of a set of int
 *
 *	\param simplexiter : int to compare to current max and min
 *
 */
void BendersBase::bound_simplex_iter(int simplexiter)
{
	if (_data.maxsimplexiter < simplexiter)
	{
		_data.maxsimplexiter = simplexiter;
	}
	if (_data.minsimplexiter > simplexiter)
	{
		_data.minsimplexiter = simplexiter;
	}
}

/*!
 *  \brief Update stopping criterion
 *
 *  Method updating the stopping criterion and reinitializing some datas
 *
 */
bool BendersBase::stopping_criterion()
{
	_data.deletedcut = 0;
	_data.maxsimplexiter = 0;
	_data.minsimplexiter = std::numeric_limits<int>::max();
	if (_data.elapsed_time > _options.TIME_LIMIT)
		_data.stopping_criterion = StoppingCriterion::timelimit;
	else if ((_options.MAX_ITERATIONS != -1) && (_data.it > _options.MAX_ITERATIONS))
		_data.stopping_criterion = StoppingCriterion::max_iteration;
	else if (_data.lb + _options.ABSOLUTE_GAP >= _data.best_ub)
		_data.stopping_criterion = StoppingCriterion::absolute_gap;
	else if (((_data.best_ub - _data.lb) / _data.best_ub) <= _options.RELATIVE_GAP)
		_data.stopping_criterion = StoppingCriterion::relative_gap;

	return _data.stopping_criterion != StoppingCriterion::empty;
}

/*!
 *  \brief Update trace of the Benders for the current iteration
 *
 *  Fonction to store the current Benders data in the trace
 */
void BendersBase::update_trace()
{
	_trace[_data.it - 1]->_lb = _data.lb;
	_trace[_data.it - 1]->_ub = _data.ub;
	_trace[_data.it - 1]->_bestub = _data.best_ub;
	_trace[_data.it - 1]->_x0 = PointPtr(new Point(_data.x0));
	_trace[_data.it - 1]->_max_invest = PointPtr(new Point(_data.max_invest));
	_trace[_data.it - 1]->_min_invest = PointPtr(new Point(_data.min_invest));
	_trace[_data.it - 1]->_deleted_cut = _data.deletedcut;
	_trace[_data.it - 1]->_time = _data.timer_master;
	_trace[_data.it - 1]->_nbasis = _data.nbasis;
	_trace[_data.it - 1]->_invest_cost = _data.invest_cost;
	_trace[_data.it - 1]->_operational_cost = _data.slave_cost;
	_trace[_data.it - 1]->_valid = true;
}

/*!
 *  \brief Check if every slave has been solved to optimality
 *
 *  \param all_package : storage of each slaves status
 *  \param data : BendersData used to get master solving status
 */
void BendersBase::check_status(AllCutPackage const &all_package) const
{
	if (_data.master_status != SOLVER_STATUS::OPTIMAL)
	{
		LOG(INFO) << "Master status is " << _data.master_status << std::endl;
		throw InvalidSolverStatusException("Master status is " + std::to_string(_data.master_status));
	}
	for (const auto &package : all_package)
	{
		for (const auto &kvp : package)
		{
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			if (handler->get_int(LPSTATUS) != SOLVER_STATUS::OPTIMAL)
			{
				std::stringstream stream;
				stream << "Slave " << kvp.first << " status is " << handler->get_int(LPSTATUS);
				LOG(INFO) << stream.str() << std::endl;

				throw InvalidSolverStatusException(stream.str());
			}
		}
	}
}

/*!
 *  \brief Solve and get optimal variables of the Master Problem
 *
 *  Method to solve and get optimal variables of the Master Problem and update upper and lower bound
 *
 */
void BendersBase::get_master_value()
{
	Timer timer_master;
	_data.alpha_i.resize(_data.nslaves);
	if (_options.BOUND_ALPHA)
	{
		_master->fix_alpha(_data.best_ub);
	}
	_master->solve(_data.master_status, _options.OUTPUTROOT);
	_master->get(_data.x0, _data.alpha, _data.alpha_i); /*Get the optimal variables of the Master Problem*/
	_master->get_value(_data.lb);						/*Get the optimal value of the Master Problem*/

	_data.invest_cost = _data.lb - _data.alpha;

	for (auto pairIdName : _master->_id_to_name)
	{
		_master->_solver->get_ub(&_data.max_invest[pairIdName.second], pairIdName.first, pairIdName.first);
		_master->_solver->get_lb(&_data.min_invest[pairIdName.second], pairIdName.first, pairIdName.first);
	}

	_data.ub = _data.invest_cost;
	_data.timer_master = timer_master.elapsed();
}

/*!
 *  \brief Solve and store optimal variables of all Slaves Problems
 *
 *  Method to solve and store optimal variables of all Slaves Problems after fixing trial values
 *
 *  \param slave_cut_package : map storing for each slave its cut
 *
 *  \param map_slaves : map linking each problem name to its problem
 *
 *  \param data : data containing trial values
 *
 *  \param options : set of parameters
 */
void BendersBase::get_slave_cut(SlaveCutPackage &slave_cut_package)
{
	for (auto &kvp : _map_slaves)
	{
		Timer timer_slave;
		WorkerSlavePtr &ptr(kvp.second);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		ptr->fix_to(_data.x0);
		ptr->solve(handler->get_int(LPSTATUS), _options.OUTPUTROOT);

		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
		ptr->get_splex_num_of_ite_last(handler->get_int(SIMPLEXITER));
		handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
		slave_cut_package[kvp.first] = *slave_cut_data;
	}
}

/*!
 *  \brief Add cut to Master Problem and store the cut in a set
 *
 *  Method to add cut from a slave to the Master Problem and store this cut in a map linking each slave to its set of cuts.
 *
 *  \param all_package : vector storing all cuts information for each slave problem
 *
 */
void BendersBase::compute_cut(AllCutPackage const &all_package)
{
	for (int i(0); i < all_package.size(); i++)
	{
		for (auto const &itmap : all_package[i])
		{
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_dbl(ALPHA_I) = _data.alpha_i[_problem_to_id[itmap.first]];
			_data.ub += handler->get_dbl(SLAVE_COST);
			SlaveCutTrimmer cut(handler, _data.x0);

			_master->add_cut_slave(_problem_to_id[itmap.first], handler->get_subgradient(), _data.x0, handler->get_dbl(SLAVE_COST));
			_all_cuts_storage[itmap.first].insert(cut);
			_trace[_data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;

			bound_simplex_iter(handler->get_int(SIMPLEXITER));
		}
	}
}

/*!
 *  \brief Add aggregated cut to Master Problem and store it in a set
 *
 *  Method to add aggregated cut from slaves to Master Problem and store it in a map linking each slave to its set of non-aggregated cut
 *
 *  \param all_package : vector storing all cuts information for each slave problem
 */
void BendersBase::compute_cut_aggregate(AllCutPackage const &all_package)
{
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++)
	{
		for (auto const &itmap : all_package[i])
		{
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			_data.ub += handler->get_dbl(SLAVE_COST);
			rhs += handler->get_dbl(SLAVE_COST);

			compute_cut_val(handler, _data.x0, s);

			SlaveCutTrimmer cut(handler, _data.x0);

			_all_cuts_storage.find(itmap.first)->second.insert(cut);
			_trace[_data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;

			bound_simplex_iter(handler->get_int(SIMPLEXITER));
		}
	}
	_master->add_cut(s, _data.x0, rhs);
}

void BendersBase::compute_cut_val(const SlaveCutDataHandlerPtr &handler, const Point &x0, Point &s) const
{
	for (auto const &var : x0)
	{
		if (handler->get_subgradient().find(var.first) != handler->get_subgradient().end())
		{
			s[var.first] += handler->get_subgradient().find(var.first)->second;
		}
	}
}
/*!
 *  \brief Add cuts in master problem
 *
 *	Add cuts in master problem according to the selected option
 *
 *  \param all_package : storage of every slave information
 *
 *  \param slave_cut_id : map linking each slaves to their cuts ids in the master problem
 *
 *  \param dynamic_aggregate_cuts : vector of tuple storing cut information (rhs, x0, subgradient)
 */
void BendersBase::build_cut_full(AllCutPackage const &all_package)
{
	check_status(all_package);
	if (_options.AGGREGATION)
	{
		compute_cut_aggregate(all_package);
	}
	else
	{
		compute_cut(all_package);
	}
}

LogData BendersBase::build_log_data_from_data() const
{
	auto logData = defineLogDataFromBendersDataAndTrace(_data, _trace);
	logData.optimality_gap = _options.ABSOLUTE_GAP;
	logData.relative_gap = _options.RELATIVE_GAP;
	logData.max_iterations = _options.MAX_ITERATIONS;
	return logData;
}

void BendersBase::post_run_actions() const
{
	LogData logData = build_log_data_from_data();

	_logger->log_stop_criterion_reached(_data.stopping_criterion);
	_logger->log_at_ending(logData);

	_writer->end_writing(output_data());
}

Output::IterationsData BendersBase::output_data() const
{
	Output::IterationsData iterations_data;
	Output::Iterations iters;
	iterations_data.nbWeeks_p = _nbWeeks;
	// Iterations
	for (auto masterDataPtr_l : _trace)
	{
		if (masterDataPtr_l->_valid)
		{
			iters.push_back(iteration(masterDataPtr_l));
		}
	}
	iterations_data.iters = iters;
	iterations_data.solution_data = solution();
	iterations_data.elapsed_time = _data.elapsed_time;
	return iterations_data;
}

Output::Iteration BendersBase::iteration(const WorkerMasterDataPtr &masterDataPtr_l) const
{
	Output::Iteration iteration;
	iteration.time = masterDataPtr_l->_time;
	iteration.lb = masterDataPtr_l->_lb;
	iteration.ub = masterDataPtr_l->_ub;
	iteration.best_ub = masterDataPtr_l->_bestub;
	iteration.optimality_gap = masterDataPtr_l->_bestub - masterDataPtr_l->_lb;
	iteration.relative_gap = (masterDataPtr_l->_bestub - masterDataPtr_l->_lb) / masterDataPtr_l->_bestub;
	iteration.investment_cost = masterDataPtr_l->_invest_cost;
	iteration.operational_cost = masterDataPtr_l->_operational_cost;
	iteration.overall_cost = masterDataPtr_l->_invest_cost + masterDataPtr_l->_operational_cost;
	iteration.candidates = candidates_data(masterDataPtr_l);
	return iteration;
}
Output::CandidatesVec BendersBase::candidates_data(const WorkerMasterDataPtr &masterDataPtr_l) const
{
	Output::CandidatesVec candidates_vec;
	for (const auto &pairNameValue_l : masterDataPtr_l->get_point())
	{
		Output::CandidateData candidate_data;
		candidate_data.name = pairNameValue_l.first;
		candidate_data.invest = pairNameValue_l.second;
		candidate_data.min = masterDataPtr_l->get_min_invest()[pairNameValue_l.first];
		candidate_data.max = masterDataPtr_l->get_max_invest()[pairNameValue_l.first];
		candidates_vec.push_back(candidate_data);
	}

	return candidates_vec;
}

Output::SolutionData BendersBase::solution() const
{
	Output::SolutionData solution_data;
	solution_data.nbWeeks_p = _nbWeeks;
	solution_data.best_it = _data.best_it;
	solution_data.problem_status = status_from_criterion();
	size_t bestItIndex_l = _data.best_it - 1;

	if (bestItIndex_l < _trace.size())
	{
		solution_data.solution = iteration(_trace[bestItIndex_l]);
		solution_data.solution.optimality_gap = _data.best_ub - _data.lb;
		solution_data.solution.relative_gap = solution_data.solution.optimality_gap / _data.best_ub;
		solution_data.stopping_criterion = criterion_to_str(_data.stopping_criterion);
	}
	return solution_data;
}

std::string BendersBase::status_from_criterion() const
{
	switch (_data.stopping_criterion)
	{
	case StoppingCriterion::absolute_gap:
	case StoppingCriterion::relative_gap:
	case StoppingCriterion::max_iteration:
	case StoppingCriterion::timelimit:
		return Output::STATUS_OPTIMAL_C;

	default:
		return Output::STATUS_ERROR_C;
	}
}

/*!
 *  \brief Get path to slave problem mps file from options
 */
std::string BendersBase::get_slave_path(std::string const &slave_name) const
{
	return (Path(_options.INPUTROOT) / (slave_name + MPS_SUFFIX)).get_str();
}

/*!
 *  \brief Return slave weight value
 *
 *  \param nslaves : total number of slaves
 *
 *  \param name : slave name
 */
double BendersBase::slave_weight(int nslaves, std::string const &name) const
{
	if (_options.SLAVE_WEIGHT == SLAVE_WEIGHT_UNIFORM_CST_STR)
	{
		return 1 / static_cast<double>(nslaves);
	}
	else if (_options.SLAVE_WEIGHT == SLAVE_WEIGHT_CST_STR)
	{
		double const weight(_options.SLAVE_WEIGHT_VALUE);
		return 1 / weight;
	}
	else
	{
		return _options.weights.find(name)->second;
	}
}

/*!
 *  \brief Get path to master problem mps file from options
 */
std::string BendersBase::get_master_path() const
{
	return (Path(_options.INPUTROOT) / (_options.MASTER_NAME + MPS_SUFFIX)).get_str();
}

/*!
 *  \brief Get path to structure txt file from options
 */
std::string BendersBase::get_structure_path() const
{
	return (Path(_options.INPUTROOT) / _options.STRUCTURE_FILE).get_str();
}