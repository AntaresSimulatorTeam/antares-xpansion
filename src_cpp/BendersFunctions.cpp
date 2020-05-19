#include "BendersFunctions.h"


/*!
*  \brief Initialize set of data used in the loop
*
*  \param data : Benders data
*/
void init(BendersData & data) {
	std::srand(time(NULL));
	data.nbasis = 0;
	data.lb = -1e20;
	data.ub = +1e20;
	data.best_ub = +1e20;
	data.stop = false;
	data.it = 0;
	data.alpha = 0;
	data.invest_cost = 0;
	data.deletedcut = 0;
	data.maxsimplexiter = 0;
	data.minsimplexiter = std::numeric_limits<int>::max();
}

/*!
*  \brief Initialize Benders log
*
*  Method to initialize Benders log by printing each column title
*
*  \param stream : output to print log
*
* \param log_level : level of log precision (from 1 to 3)
*/
void init_log(std::ostream&stream, int const log_level) {
	stream << std::setw(10) << "ITE";
	stream << std::setw(20) << "LB";
	stream << std::setw(20) << "UB";
	stream << std::setw(20) << "BESTUB";
	stream << std::setw(15) << "GAP";

	if (log_level > 1) {
		stream << std::setw(15) << "MINSIMPLEX";
		stream << std::setw(15) << "MAXSIMPLEX";
	}

	if (log_level > 2) {
		stream << std::setw(15) << "DELETEDCUT";
		stream << std::setw(15) << "TIMEMASTER";
		stream << std::setw(15) << "TIMESLAVES";
	}
	stream << std::endl;
}

/*!
*  \brief Print iteration log
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*
* \param data : data to print
*
* \param log_level : level of log precision (from 1 to 3)
*/
void print_log(std::ostream&stream, BendersData const & data, int const log_level) {

	stream << std::setw(10) << data.it;
	if (data.lb == -1e20)
		stream << std::setw(20) << "-INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.lb;
	if (data.ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.ub;
	if (data.best_ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.best_ub;
		stream << std::setw(15) << std::scientific << std::setprecision(2) << data.best_ub - data.lb;

	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
		stream << std::setw(15) << std::setprecision(2) << data.timer_master - data.timer_slaves;
		stream << std::setw(15) << std::setprecision(2) << data.timer_slaves;
	}
	stream << std::endl;

}

/*!
*  \brief Print the trace of the Benders algorithm in a csv file
*
*  Method to print trace of the Benders algorithm in a csv file
*
* \param stream : stream to print the output
*
* \param problem_to_id : map linking each problem name to its id
*
* \param data : set of Benders data
*
* \param options : set of parameters
*/
void print_csv(BendersTrace & trace, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options) {
	std::string const output(options.OUTPUTROOT + PATH_SEPARATOR + options.CSV_NAME + ".csv");
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file)
	{
		file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;alpha_i;deletedcut;time;basis;" << std::endl;
		Point xopt;
		int const nite(data.it);
		xopt = trace[nite - 1]->get_point();
		file << 1 << ";";
		print_master_csv(file, trace[0], xopt, options.MASTER_NAME, data.nslaves);
		for (auto & kvp : trace[0]->_cut_trace) {
			SlaveCutDataHandler const handler(kvp.second);
			file << 1 << ";";
			print_cut_csv(file, handler, kvp.first, problem_to_id[kvp.first]);
		}
		for (int i(1); i < nite; i++) {
			file << i + 1 << ";";
			print_master_csv(file, trace[i], trace[i - 1]->get_point(), options.MASTER_NAME, data.nslaves);
			for (auto & kvp : trace[i]->_cut_trace) {
				SlaveCutDataHandler const handler(kvp.second);
				file << i + 1 << ";";
				print_cut_csv(file, handler, kvp.first, problem_to_id[kvp.first]);
			}
		}
		file.close();
	}
	else {
		std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
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
void print_master_csv(std::ostream&stream, WorkerMasterDataPtr & trace, Point const & xopt, std::string const & name, int const nslaves) {
	stream << "Master" << ";";
	stream << name << ";";
	stream << nslaves << ";";
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
void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, std::string const & name, int const islaves) {
	stream << "Slave" << ";";
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
*  \brief Print the optimal solution of the problem
*
*  Method to print the optimal solution of the problem
*
* \param stream : stream to print the output
*
* \param point : point to print
*
* \param filter_non_zero : either if zeros coordinates need to be printed or not
*/
void print_solution(std::ostream&stream, Point const & point, bool const filter_non_zero) {
	stream << std::endl;
	stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
	stream << "*                                                                                               *" << std::endl;
	stream << "*                                      Investment solution                                      *" << std::endl;
	stream << "*                                                                                               *" << std::endl;
	stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
	stream << "|                                                                                               |" << std::endl;
	for (auto const & kvp : point) {
		if (!filter_non_zero || std::fabs(kvp.second) > 1e-10) {
			stream << "|  " << std::setw(70) << std::left << kvp.first;
			stream << " = ";
			stream << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
			stream << "|" << std::endl;
		}
	}
	stream << "|_______________________________________________________________________________________________|" << std::endl;
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
void print_active_cut(ActiveCutStorage const & active_cuts, BendersOptions const & options) {
	std::string output(options.OUTPUTROOT + PATH_SEPARATOR + "active_cut_output.csv");
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file)
	{
		file << "Ite;Slave;CutNumber;IsActive;" << std::endl;
		for (int i(0); i < active_cuts.size(); i++) {
			file << std::get<0>(active_cuts[i]) << ";";
			file << std::get<1>(active_cuts[i]) << ";";
			file << std::get<2>(active_cuts[i]) << ";";
			file << std::get<3>(active_cuts[i]) << ";" << std::endl;
		}
		file.close();
	}
	else {
		std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
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
void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0) {
	if (best_ub > ub) {
		best_ub = ub;
		bestx = x0;
	}
}

/*!
*	\brief Update maximum and minimum of a set of int
*
*	\param simplexiter : int to compare to current max and min
*
*   \param data : data containing current maximum and minimum integer
*/
void bound_simplex_iter(int simplexiter, BendersData & data) {
	if (data.maxsimplexiter < simplexiter) {
		data.maxsimplexiter = simplexiter;
	}
	if (data.minsimplexiter > simplexiter) {
		data.minsimplexiter = simplexiter;
	}
}

/*!
*  \brief Update stopping criterion
*
*  Method updating the stopping criterion and reinitializing some datas
*
*  \param data : data containing benders information
*
*  \param options : stopping parameters
*/
bool stopping_criterion(BendersData & data, BendersOptions const & options) {
	data.deletedcut = 0;
	data.maxsimplexiter = 0;
	data.minsimplexiter = std::numeric_limits<int>::max();
	return(((options.MAX_ITERATIONS != -1) && (data.it > options.MAX_ITERATIONS)) || (data.lb + options.GAP >= data.best_ub));
}

/*!
*  \brief Update trace of the Benders for the current iteration
*
*  Fonction to store the current Benders data in the trace
*
*  \param trace : vector keeping data for each iteration
*
*  \param data : data to store
*/
void update_trace(BendersTrace & trace, BendersData const & data) {
	trace[data.it - 1]->_lb = data.lb;
	trace[data.it - 1]->_ub = data.ub;
	trace[data.it - 1]->_bestub = data.best_ub;
	trace[data.it - 1]->_x0 = PointPtr(new Point(data.x0));
	trace[data.it - 1]->_deleted_cut = data.deletedcut;
	trace[data.it - 1]->_time = data.timer_master;
	trace[data.it - 1]->_nbasis = data.nbasis;

}

/*!
*  \brief Check if every slave has been solved to optimality
*
*  \param all_package : storage of each slaves status
*/
void check_status(AllCutPackage const & all_package, BendersData const & data) {
	if (data.master_status != 1) {
		std::cout << "Master status is " << data.master_status << std::endl;
		exit(0);
	}
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			if (handler->get_int(LPSTATUS) != 1) {
				std::cout << "Slave " << kvp.first << " status is " << handler->get_int(LPSTATUS) << std::endl;
				exit(0);
			}
		}
	}
}

/*!
*  \brief Solve and get optimal variables of the Master Problem
*
*  Method to solve and get optimal variables of the Master Problem and update upper and lower bound
*
*  \param master : pointer to the master problem
*
*  \param data : benders data to update with master optimal solution
*/
void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options) {
	Timer timer_master;
	data.alpha_i.resize(data.nslaves);
	if (options.BOUND_ALPHA) {
		master->fix_alpha(data.best_ub);
	}
	master->solve(data.master_status);
	master->get(data.x0, data.alpha, data.alpha_i); /*Get the optimal variables of the Master Problem*/
	master->get_value(data.lb); /*Get the optimal value of the Master Problem*/
	data.invest_cost = data.lb - data.alpha;
	if (!options.RAND_AGGREGATION) {
		data.ub = data.invest_cost;
	}
	data.timer_master = timer_master.elapsed();
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
void get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data) {
	for (auto & kvp : map_slaves) {
		Timer timer_slave;
		WorkerSlavePtr & ptr(kvp.second);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		ptr->fix_to(data.x0);
		ptr->solve(handler->get_int(LPSTATUS));
		if (options.BASIS) {
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
void get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, StrVector const & random_slaves, BendersOptions const & options, BendersData const & data) {
	for (int i(0); i < data.nrandom; i++){
		Timer timer_slave;
		std::string const name_slave(random_slaves[i]);
		WorkerSlavePtr & ptr(map_slaves[name_slave]);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		ptr->fix_to(data.x0);
		ptr->solve(handler->get_int(LPSTATUS));
		if (options.BASIS) {
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
*  \param master : pointer to thte master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param slave_cut_id : map linking each cut to its id in the master problem
*
*  \param data : Benders data
*
*  \param options : set of parameters
*
*/
void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, BendersTrace & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id) {
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[itmap.first]];
			data.ub += handler->get_dbl(SLAVE_COST);
			SlaveCutTrimmer cut(handler, data.x0);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			else {
				master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
				if (options.ACTIVECUTS) {
					slave_cut_id[itmap.first].push_back(master->get_number_constraint());
				}
				all_cuts_storage[itmap.first].insert(cut);
			}
			if (options.TRACE) {
				trace[data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
			}
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}
}

/*!
*  \brief Add aggregated cut to Master Problem and store it in a set
*
*  Method to add aggregated cut from slaves to Master Problem and store it in a map linking each slave to its set of non-aggregated cut
*
*  \param all_package : vector storing all cuts information for each slave problem
*
*  \param master : pointer to thte master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param data : Benders data
*
*  \param options : set of parameters
*/
void sort_cut_slave_aggregate(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, BendersTrace & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options) {
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			data.ub += handler->get_dbl(SLAVE_COST);
			rhs += handler->get_dbl(SLAVE_COST);
			for (auto const & var : data.x0) {
				s[var.first] += handler->get_subgradient()[var.first];
			}
			SlaveCutTrimmer cut(handler, data.x0);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			all_cuts_storage.find(itmap.first)->second.insert(cut);
			if (options.TRACE) {
				trace[data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
			}
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}
	master->add_cut(s, data.x0, rhs);
}


/*!
*  \brief Add the random cuts in master problem
*
*	Add the random cuts in master problem
*
*  \param master : pointer to master problem
*
*  \param all_package : storage of every slave information
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersTrace & trace, BendersOptions & options, BendersData & data) {
	int nboundslaves(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			master->add_cut_slave(problem_to_id[kvp.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
			if (handler->get_dbl(SLAVE_COST) <= options.GAP + handler->get_dbl(ALPHA_I)) {
				nboundslaves++;
			}
			if (options.TRACE) {
				trace[data.it - 1]->_cut_trace[kvp.first] = slave_cut_data;
			}
		}
	}
	if (nboundslaves == options.RAND_AGGREGATION) {
		options.RAND_AGGREGATION = 0;
	}
}


/*!
*  \brief Add cuts in master problem
*
*	Add cuts in master problem according to the selected option
*
*  \param master : pointer to master problem
*
*  \param all_package : storage of every slave information
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param slave_cut_id : map linking each slaves to their cuts ids in the master problem
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param dynamic_aggregate_cuts : vector of tuple storing cut information (rhs, x0, subgradient)
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersTrace & trace, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, DynamicAggregateCuts & dynamic_aggregate_cuts, BendersData & data, BendersOptions & options) {
	check_status(all_package, data);
	if (!options.AGGREGATION && !options.RAND_AGGREGATION) {
		sort_cut_slave(all_package, master, problem_to_id, trace, all_cuts_storage, data, options, slave_cut_id);
	}
	else if (options.AGGREGATION) {
		sort_cut_slave_aggregate(all_package, master, problem_to_id, trace, all_cuts_storage, data, options);
	}
	else if (options.RAND_AGGREGATION) {
		add_random_cuts(master, all_package, problem_to_id, trace, options, data);
	}
	if (options.THRESHOLD_AGGREGATION > 1) {
		dynamic_aggregation(master, dynamic_aggregate_cuts, all_package, problem_to_id, data, options);
	}
	if (options.THRESHOLD_ITERATION > 1) {
		dynamic_iteration(master, dynamic_aggregate_cuts, all_package, problem_to_id, data, options);
	}
}

/*!
*  \brief Store and gather cuts for dynamic aggregation
*
*  \param master : pointer to master problem
*
*  \param dynamic_aggregate_cuts : vector of tuple storing cut information (s, s*x0, rhs)
*
*  \param all_package : storage of every slave information
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param data : set of Benders data
*
*  \param options : set of Benders options
*/
void dynamic_aggregation(WorkerMasterPtr & master, DynamicAggregateCuts & dynamic_aggregate_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options) {
	if (data.it == 1) {
		for (int i(0); i < options.THRESHOLD_AGGREGATION; i++) {
			Point s;
			double sx0(0);
			double rhs(0);
			dynamic_aggregate_cuts.push_back(std::tuple<Point, double, double>(s, sx0, rhs));
		}
	}
	store_current_aggregate_cut(dynamic_aggregate_cuts, all_package, problem_to_id, data, options);
	if (data.it % options.THRESHOLD_AGGREGATION == 0) {
		gather_cut(dynamic_aggregate_cuts, master, options, options.THRESHOLD_AGGREGATION * data.nslaves, problem_to_id);
	}
}

/*!
*  \brief Store and gather cuts for dynamic aggregation by iteration
*
*  \param master : pointer to master problem
*
*  \param dynamic_aggregate_cuts : vector of tuple storing cut information (s, s*x0, rhs)
*
*  \param all_package : storage of every slave information
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param data : set of Benders data
*
*  \param options : set of Benders options
*/
void dynamic_iteration(WorkerMasterPtr & master, DynamicAggregateCuts & dynamic_aggregate_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options) {
	if (data.it == 1) {
		for (int i(0); i < data.nslaves; i++) {
			Point s;
			double sx0(0);
			double rhs(0);
			dynamic_aggregate_cuts.push_back(std::tuple<Point, double, double>(s, sx0, rhs));
		}
	}
	store_iter_aggregate_cut(dynamic_aggregate_cuts, all_package, problem_to_id, data.x0, data, options);
	if (data.it % options.THRESHOLD_ITERATION == 0) {
		gather_cut(dynamic_aggregate_cuts, master, options, options.THRESHOLD_ITERATION * data.nslaves, problem_to_id);
	}
}

/*!
*  \brief Store the current aggregate cuts for further aggregation
*
*	Store the current aggregate cuts in case of partial aggregation
*
*  \param dynamic_cuts : vector of tuple storing cut information (s, s*x0, rhs)
*
*  \param all_package : storage of every slave information
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param x0 : trial values fixed in each slave
*
*  \param data : set of Benders data
*
*  \param options : set of Benders options
*/
void store_current_aggregate_cut(DynamicAggregateCuts & dynamic_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersData const & data, BendersOptions const & options) {
	int const nite = data.it % options.THRESHOLD_AGGREGATION;
	if (nite == 1) {
		for (int i(0); i < options.THRESHOLD_AGGREGATION; i++) {
			std::get<0>(dynamic_cuts[i]).clear();
			std::get<1>(dynamic_cuts[i]) = 0;
			std::get<2>(dynamic_cuts[i]) = 0;
		}
	}
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			for (auto const & kvp : data.x0) {
				std::get<0>(dynamic_cuts[nite])[kvp.first] += handler->get_subgradient()[kvp.first];
				std::get<1>(dynamic_cuts[nite]) += handler->get_subgradient()[kvp.first] * kvp.second;
			}
			std::get<2>(dynamic_cuts[nite]) += handler->get_dbl(SLAVE_COST);
		}
	}
}

/*!
*  \brief Store the current aggregate cuts for further aggregation by iteration
*
*	Store the current aggregate cuts in case of partial aggregation
*
*  \param dynamic_cuts : vector of tuple storing cut information (s, s*x0, rhs)
*
*  \param all_package : storage of every slave information
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param x0 : trial values fixed in each slave
*
*  \param data : set of Benders data
*
*  \param options : set of Benders options
*/
void store_iter_aggregate_cut(DynamicAggregateCuts & dynamic_cuts, AllCutPackage const & all_package, Str2Int & problem_to_id, Point const & x0, BendersData const & data, BendersOptions const & options) {
	if (data.it % options.THRESHOLD_ITERATION == 1) {
		for (int i(0); i < data.nslaves; i++) {
			std::get<0>(dynamic_cuts[i]).clear();
			std::get<1>(dynamic_cuts[i]) = 0;
			std::get<2>(dynamic_cuts[i]) = 0;
		}
	}
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			for (auto const & kvp : handler->get_subgradient()) {
				std::get<0>(dynamic_cuts[problem_to_id[itmap.first]])[kvp.first] += kvp.second / options.THRESHOLD_ITERATION;
				std::get<1>(dynamic_cuts[problem_to_id[itmap.first]]) += kvp.second * x0.find(kvp.first)->second / options.THRESHOLD_ITERATION;
			}
			std::get<2>(dynamic_cuts[problem_to_id[itmap.first]]) += handler->get_dbl(SLAVE_COST) / options.THRESHOLD_ITERATION;
		}
	}
}

/*!
*  \brief Add all the stored cuts in case of partial aggregation
*
*	Delete every previous simple cut and replace them with the aggregated ones
*
*  \param dynamic_cuts : vector of tuple storing cut information (rhs, x0, subgradient)
*
*  \param master : pointer to master problem
*
*  \param it : number of iteration
*
*  \param nconstraints : number of previous added constraints to delete
*/
void gather_cut(DynamicAggregateCuts & dynamic_cuts, WorkerMasterPtr & master, BendersOptions const & options, int const nconstraints, Str2Int const & problem_to_id) {
	master->delete_constraint(nconstraints);
	if (options.THRESHOLD_AGGREGATION > 1) {
		for (int i(0); i < dynamic_cuts.size(); i++) {
			master->add_dynamic_cut(std::get<0>(dynamic_cuts[i]), std::get<1>(dynamic_cuts[i]), std::get<2>(dynamic_cuts[i]));
		}
	}
	else if (options.THRESHOLD_ITERATION > 1) {
		for (auto const & kvp : problem_to_id) {
			master->add_cut_by_iter(kvp.second, std::get<0>(dynamic_cuts[kvp.second]), std::get<1>(dynamic_cuts[kvp.second]), std::get<2>(dynamic_cuts[kvp.second]));
		}
	}
}

/*!
*  \brief Get all slaves basis from a map
*
*  Fonction to get all slaves basis
*
*  \param simplex_basis_package : map linking each slave to its current simplex basis
*
*  \param map_slaves : map linking each slaves names to their problem
*/
void get_slave_basis(SimplexBasisPackage & simplex_basis_package, SlavesMapPtr & map_slaves) {
	for (auto & kvp : map_slaves) {
		WorkerSlavePtr & ptr(kvp.second);
		simplex_basis_package[kvp.first] = ptr->get_basis();
	}
}

/*!
*  \brief Store all slaves basis in a set
*
*  Fonction to store and sort all slaves basis in a set
*
*  \param all_basis_package : vector of slaves basis
*
*  \param problem_to_id : map linking each problem name to its id
*
*  \param basis_storage : set storing every basis
*
*  \param data : set of Benders data
*/
void sort_basis(AllBasisPackage const & all_basis_package, Str2Int & problem_to_id, SimplexBasisStorage & basis_storage, BendersData & data) {
	for (int i(0); i < all_basis_package.size(); i++) {
		for (auto const & itmap : all_basis_package[i]) {
			SimplexBasisPtr basis(new SimplexBasis(itmap.second));
			SimplexBasisHandler handler(basis);
			basis_storage.insert(handler);
		}
	}
	data.nbasis = basis_storage.size();
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
void update_active_cuts(WorkerMasterPtr & master, ActiveCutStorage & active_cuts, SlaveCutId & cut_id, int const it) {
	DblVector dual;
	master->get_dual_values(dual);
	for (auto & kvp : cut_id) {
		for (int i(0); i < kvp.second.size(); i++) {
			active_cuts.push_back(std::make_tuple(it, kvp.first, i + 1, (dual[kvp.first[i]] != 0)));
			//	}
			//}
		}
	}
}

