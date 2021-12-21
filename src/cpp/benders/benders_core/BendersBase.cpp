#include "BendersBase.h"
#include "launcher.h"
#include "solver_utils.h"

#include "glog/logging.h"

BendersBase::BendersBase(BendersOptions const &options, Logger &logger, Writer writer) : _options(options), _logger(logger), _writer(writer) {}

BendersBase::~BendersBase()
{
}
/*!
 *  \brief Initialize set of data used in the loop
 */
void BendersBase::init_data()
{
	std::srand(time(NULL));
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
	std::string const output(_options.OUTPUTROOT + PATH_SEPARATOR + _options.CSV_NAME + ".csv");
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file)
	{
		file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;alpha_i;deletedcut;time;basis;" << std::endl;
		int const nite = _trace.size();
		for (int i = 0; i < nite; i++)
		{
			if (_trace[i]->_valid)
			{
				Point xopt;
				// Write first problem : use result of best iteration
				if (i == 0)
				{
					int best_it_index = _data.best_it - 1;
					if (best_it_index >= 0 && _trace.size() > best_it_index)
					{
						xopt = _trace[best_it_index]->get_point();
					}
				}
				else
				{
					xopt = _trace[i - 1]->get_point();
				}
				print_master_and_cut(file, i + 1, _trace[i], xopt);
			}
		}
		file.close();
	}
	else
	{
		LOG(INFO) << "Impossible to open the .csv file" << std::endl;
	}
}

void BendersBase::print_master_and_cut(std::ostream &file, int ite, WorkerMasterDataPtr &trace, Point const &xopt)
{
	file << ite << ";";

	print_master_csv(file, trace, xopt);

	for (auto &kvp : trace->_cut_trace)
	{
		SlaveCutDataHandler const handler(kvp.second);
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
void BendersBase::print_master_csv(std::ostream &stream, WorkerMasterDataPtr &trace, Point const &xopt)
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
void BendersBase::print_cut_csv(std::ostream &stream, SlaveCutDataHandler const &handler, std::string const &name, int const islaves)
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
 *  \brief Write in a csv file every active cut at every iteration
 *
 *	Write in a csv file every active cut for every slave for all iterations
 *
 *  \param active_cuts : vector of tuple storing which cut is active or not
 *
 *  \param options : set of parameters
 */
void BendersBase::print_active_cut()
{
	std::string output(_options.OUTPUTROOT + PATH_SEPARATOR + "active_cut_output.csv");
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file)
	{
		file << "Ite;Slave;CutNumber;IsActive;" << std::endl;
		for (int i(0); i < _active_cuts.size(); i++)
		{
			file << std::get<0>(_active_cuts[i]) << ";";
			file << std::get<1>(_active_cuts[i]) << ";";
			file << std::get<2>(_active_cuts[i]) << ";";
			file << std::get<3>(_active_cuts[i]) << ";" << std::endl;
		}
		file.close();
	}
	else
	{
		LOG(INFO) << "Impossible d'ouvrir le fichier .csv" << std::endl;
	}
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
void BendersBase::check_status(AllCutPackage const &all_package)
{
	if (_data.master_status != SOLVER_STATUS::OPTIMAL)
	{
		LOG(INFO) << "Master status is " << _data.master_status << std::endl;
		throw InvalidSolverStatusException("Master status is " + std::to_string(_data.master_status));
	}
	for (int i(0); i < all_package.size(); i++)
	{
		for (auto const &kvp : all_package[i])
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
	_master->solve(_data.master_status, _options);
	_master->get(_data.x0, _data.alpha, _data.alpha_i); /*Get the optimal variables of the Master Problem*/
	_master->get_value(_data.lb);						/*Get the optimal value of the Master Problem*/

	_data.invest_cost = _data.lb - _data.alpha;

	for (auto pairIdName : _master->_id_to_name)
	{
		_master->_solver->get_ub(&_data.max_invest[pairIdName.second], pairIdName.first, pairIdName.first);
		_master->_solver->get_lb(&_data.min_invest[pairIdName.second], pairIdName.first, pairIdName.first);
	}

	if (!_options.RAND_AGGREGATION)
	{
		_data.ub = _data.invest_cost;
	}
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
		ptr->solve(handler->get_int(LPSTATUS), _options);
		if (_options.BASIS)
		{
			ptr->get_basis();
		}
		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
		ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
		handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
		slave_cut_package[kvp.first] = *slave_cut_data;
	}
}

/*!
 *  \brief Solve and store optimal variables of random Slaves Problems
 *
 *  Method to solve and store optimal variables of random Slaves Problems after fixing trial values
 *
 *  \param slave_cut_package : map storing for each slave its cut
 *
 *  \param map_slaves : map linking each problem name to its problem
 *
 *  \param data : data containing trial values
 *
 *  \param options : set of parameters
 */
void BendersBase::get_random_slave_cut(SlaveCutPackage &slave_cut_package)
{
	for (int i(0); i < _data.nrandom; i++)
	{
		Timer timer_slave;
		std::string const name_slave(_slaves[i]);
		WorkerSlavePtr &ptr(_map_slaves[name_slave]);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		ptr->fix_to(_data.x0);
		ptr->solve(handler->get_int(LPSTATUS), _options);
		if (_options.BASIS)
		{
			ptr->get_basis();
		}
		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
		ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
		handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
		slave_cut_package[name_slave] = *slave_cut_data;
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
			if (_options.DELETE_CUT && !(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end()))
			{
				_data.deletedcut++;
			}
			else
			{
				_master->add_cut_slave(_problem_to_id[itmap.first], handler->get_subgradient(), _data.x0, handler->get_dbl(SLAVE_COST));
				if (_options.ACTIVECUTS)
				{
					_slave_cut_id[itmap.first].push_back(_master->get_number_constraint());
				}
				_all_cuts_storage[itmap.first].insert(cut);
			}
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
			if (_options.DELETE_CUT && !(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end()))
			{
				_data.deletedcut++;
			}
			_all_cuts_storage.find(itmap.first)->second.insert(cut);
			_trace[_data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;

			bound_simplex_iter(handler->get_int(SIMPLEXITER));
		}
	}
	_master->add_cut(s, _data.x0, rhs);
}

/*!
 *  \brief Add the random cuts in master problem
 *
 *	Add the random cuts in master problem
 *
 *  \param all_package : storage of every slave information
 */
void BendersBase::add_random_cuts(AllCutPackage const &all_package)
{
	int nboundslaves(0);
	for (int i(0); i < all_package.size(); i++)
	{
		for (auto const &kvp : all_package[i])
		{
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			_master->add_cut_slave(_problem_to_id[kvp.first], handler->get_subgradient(), _data.x0, handler->get_dbl(SLAVE_COST));
			handler->get_dbl(ALPHA_I) = _data.alpha_i[_problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER));
			if (handler->get_dbl(SLAVE_COST) <= _options.ABSOLUTE_GAP + handler->get_dbl(ALPHA_I))
			{
				nboundslaves++;
			}
			_trace[_data.it - 1]->_cut_trace[kvp.first] = slave_cut_data;
		}
	}
	if (nboundslaves == _options.RAND_AGGREGATION)
	{
		_options.RAND_AGGREGATION = 0;
	}
}

void BendersBase::compute_cut_val(const SlaveCutDataHandlerPtr &handler, const Point &x0, Point &s)
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
	if (!_options.AGGREGATION && !_options.RAND_AGGREGATION)
	{
		compute_cut(all_package);
	}
	else if (_options.AGGREGATION)
	{
		compute_cut_aggregate(all_package);
	}
	else if (_options.RAND_AGGREGATION)
	{
		add_random_cuts(all_package);
	}
	if (_options.THRESHOLD_AGGREGATION > 1)
	{
		dynamic_aggregation(all_package);
	}
	if (_options.THRESHOLD_ITERATION > 1)
	{
		dynamic_iteration(all_package);
	}
}

/*!
 *  \brief Store and gather cuts for dynamic aggregation
 *
 *  \param all_package : storage of every slave information
 */
void BendersBase::dynamic_aggregation(AllCutPackage const &all_package)
{
	if (_data.it == 1)
	{
		for (int i(0); i < _options.THRESHOLD_AGGREGATION; i++)
		{
			Point s;
			double sx0(0);
			double rhs(0);
			_dynamic_aggregate_cuts.push_back(std::tuple<Point, double, double>(s, sx0, rhs));
		}
	}
	store_current_aggregate_cut(all_package);
	if (_data.it % _options.THRESHOLD_AGGREGATION == 0)
	{
		gather_cut();
	}
}
/*!
 *  \brief Store and gather cuts for dynamic aggregation by iteration
 *
 *  \param all_package : storage of every slave information
 */
void BendersBase::dynamic_iteration(AllCutPackage const &all_package)
{
	if (_data.it == 1)
	{
		for (int i(0); i < _data.nslaves; i++)
		{
			Point s;
			double sx0(0);
			double rhs(0);
			_dynamic_aggregate_cuts.push_back(std::tuple<Point, double, double>(s, sx0, rhs));
		}
	}
	store_iter_aggregate_cut(all_package);
	if (_data.it % _options.THRESHOLD_ITERATION == 0)
	{
		gather_cut();
	}
}

/*!
 *  \brief Add all the stored cuts in case of partial aggregation
 *
 *	Delete every previous simple cut and replace them with the aggregated ones
 *
 */
void BendersBase::gather_cut()
{

	int const nconstraints = _options.THRESHOLD_ITERATION * _data.nslaves;
	_master->delete_constraint(nconstraints);
	if (_options.THRESHOLD_AGGREGATION > 1)
	{
		for (int i(0); i < _dynamic_aggregate_cuts.size(); i++)
		{
			_master->add_dynamic_cut(std::get<0>(_dynamic_aggregate_cuts[i]), std::get<1>(_dynamic_aggregate_cuts[i]), std::get<2>(_dynamic_aggregate_cuts[i]));
		}
	}
	else if (_options.THRESHOLD_ITERATION > 1)
	{
		for (auto const &kvp : _problem_to_id)
		{
			_master->add_cut_by_iter(kvp.second, std::get<0>(_dynamic_aggregate_cuts[kvp.second]), std::get<1>(_dynamic_aggregate_cuts[kvp.second]), std::get<2>(_dynamic_aggregate_cuts[kvp.second]));
		}
	}
}
/*!
 *  \brief Store the current aggregate cuts for further aggregation
 *
 *	Store the current aggregate cuts in case of partial aggregation
 *
 *  \param all_package : storage of every slave information
 */
void BendersBase::store_current_aggregate_cut(AllCutPackage const &all_package)
{
	int const nite = _data.it % _options.THRESHOLD_AGGREGATION;
	if (nite == 1)
	{
		for (int i(0); i < _options.THRESHOLD_AGGREGATION; i++)
		{
			std::get<0>(_dynamic_aggregate_cuts[i]).clear();
			std::get<1>(_dynamic_aggregate_cuts[i]) = 0;
			std::get<2>(_dynamic_aggregate_cuts[i]) = 0;
		}
	}
	for (int i(0); i < all_package.size(); i++)
	{
		for (auto const &itmap : all_package[i])
		{
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			compute_dynamic_cut_at_iter(handler, nite);
		}
	}
}

void BendersBase::compute_dynamic_cut_at_iter(const SlaveCutDataHandlerPtr &handler, int const nite)
{
	for (auto const &kvp : _data.x0)
	{
		if (handler->get_subgradient().find(kvp.first) != handler->get_subgradient().end())
		{
			std::get<0>(_dynamic_aggregate_cuts[nite])[kvp.first] += handler->get_subgradient()[kvp.first];
			std::get<1>(_dynamic_aggregate_cuts[nite]) += handler->get_subgradient()[kvp.first] * kvp.second;
		}
	}
	std::get<2>(_dynamic_aggregate_cuts[nite]) += handler->get_dbl(SLAVE_COST);
}

/*!
 *  \brief Store the current aggregate cuts for further aggregation by iteration
 *
 *	Store the current aggregate cuts in case of partial aggregation
 *
 *  \param all_package : storage of every slave information
 */
void BendersBase::store_iter_aggregate_cut(AllCutPackage const &all_package)
{
	if (_data.it % _options.THRESHOLD_ITERATION == 1)
	{
		for (int i(0); i < _data.nslaves; i++)
		{
			std::get<0>(_dynamic_aggregate_cuts[i]).clear();
			std::get<1>(_dynamic_aggregate_cuts[i]) = 0;
			std::get<2>(_dynamic_aggregate_cuts[i]) = 0;
		}
	}
	for (int i(0); i < all_package.size(); i++)
	{
		for (auto const &itmap : all_package[i])
		{
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			for (auto const &kvp : handler->get_subgradient())
			{
				std::get<0>(_dynamic_aggregate_cuts[_problem_to_id[itmap.first]])[kvp.first] += kvp.second / _options.THRESHOLD_ITERATION;
				std::get<1>(_dynamic_aggregate_cuts[_problem_to_id[itmap.first]]) += kvp.second * _data.x0.find(kvp.first)->second / _options.THRESHOLD_ITERATION;
			}
			std::get<2>(_dynamic_aggregate_cuts[_problem_to_id[itmap.first]]) += handler->get_dbl(SLAVE_COST) / _options.THRESHOLD_ITERATION;
		}
	}
}

/*!
 *  \brief Get all slaves basis from a map
 *
 *  Fonction to get all slaves basis
 *
 *  \param simplex_basis_package : map linking each slave to its current simplex basis
 */
void BendersBase::get_slave_basis(SimplexBasisPackage &simplex_basis_package)
{
	for (auto &kvp : _map_slaves)
	{
		WorkerSlavePtr &ptr(kvp.second);
		simplex_basis_package[kvp.first] = ptr->get_basis();
	}
}

/*!
 *  \brief Store all slaves basis in a set
 *
 *  Fonction to store and sort all slaves basis in a set
 *
 *  \param all_basis_package : vector of slaves basis
 */
void BendersBase::sort_basis(AllBasisPackage const &all_basis_package)
{
	for (int i(0); i < all_basis_package.size(); i++)
	{
		for (auto const &itmap : all_basis_package[i])
		{
			SimplexBasisPtr basis(new SimplexBasis(itmap.second));
			SimplexBasisHandler handler(basis);
			_basis.insert(handler);
		}
	}
	_data.nbasis = _basis.size();
}

/*!
 *  \brief Store all cuts status at each iteration
 *
 *  Fonction to store all cuts status from master problem at each iteration
 *
 *  \param master : pointer to master problem
 *
 *  \param active_cuts : vector of tuple storing each cut status
 *
 *  \param cut_id : map linking each cut from each slave to its id in master problem
 *
 *  \param it : current iteration
 */
void BendersBase::update_active_cuts()
{
	DblVector dual;
	_master->get_dual_values(dual);
	for (auto &kvp : _slave_cut_id)
	{
		for (int i(0); i < kvp.second.size(); i++)
		{
			_active_cuts.push_back(std::make_tuple(_data.it, kvp.first, i + 1, (dual[kvp.first[i]] != 0))); //@TODO check : kvp.first is a string and indexing a vector
		}
	}
}

void BendersBase::fill_log_data_from_data(LogData &logData)
{
	logData = defineLogDataFromBendersDataAndTrace(_data, _trace);
	logData.optimality_gap = _options.ABSOLUTE_GAP;
	logData.relative_gap = _options.RELATIVE_GAP;
	logData.max_iterations = _options.MAX_ITERATIONS;
}

void BendersBase::post_run_actions(CouplingMap input)
{
	LogData logData;
	fill_log_data_from_data(logData);

	_logger->log_at_ending(logData);

	_writer->end_writing(input.size(), _trace, _data, _options.ABSOLUTE_GAP,
						 _options.RELATIVE_GAP, _options.MAX_ITERATIONS);

	std::stringstream str;
	str << "Optimization results available in : "
		<< _options.JSON_FILE;
	_logger->display_message(str.str());
}