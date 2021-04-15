#include "Benders.h"

#include "ortools_utils.h"

#include <iomanip>
#include <algorithm>
#include <random>

Benders::~Benders() {
}

/*!
*  \brief Constructor of class Benders
*
*  Method to build a Benders element, initializing each problem from a list
*
*  \param problem_list : map linking each problem name to its variables and their ids
*
*  \param options : set of options fixed by the user
*/
Benders::Benders(CouplingMap const & problem_list, BendersOptions const & options) : _options(options) {
	if (!problem_list.empty()) {
		_data.nslaves = _options.SLAVE_NUMBER;
		if (_data.nslaves < 0) {
			_data.nslaves = problem_list.size() - 1;
		}

		auto it(problem_list.begin());

		auto const it_master = problem_list.find(_options.MASTER_NAME);
		Str2Int const & master_variable(it_master->second);
		for(int i(0); i < _data.nslaves; ++it) {
			if (it != it_master) {
				_problem_to_id[it->first] = i;
				_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path(it->first), _options.slave_weight(_data.nslaves, it->first), _options));
				_slaves.push_back(it->first);
				i++;
			}
		}
		_master.reset(new WorkerMaster(master_variable, _options.get_master_path(), _options, _data.nslaves));
	}

}


/*!
*  \brief Method to free the memory used by each problem
*/
void Benders::free() {
	_master->free();
	for (auto & ptr : _map_slaves)
		ptr.second->free();
}

/*!
*  \brief Build Slave cut and store it in the Benders trace
*
*  Method to build Slaves cuts, store them in the Benders trace and add them to the Master problem
*
*/
void Benders::build_cut() {
	SlaveCutPackage slave_cut_package;
	AllCutPackage all_package;
	Timer timer_slaves;
	if (_options.RAND_AGGREGATION) {
        std::random_device rd;
        std::mt19937 g(rd());
		std::shuffle(_slaves.begin(), _slaves.end(),g);
		get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, _data);
	}
	else {
		get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
		_data.slave_cost = 0;
		for (auto pairSlavenameSlavecutdata_l : slave_cut_package)
		{
			_data.slave_cost += pairSlavenameSlavecutdata_l.second.first.second[SLAVE_COST];
		}
	}
	
	_data.timer_slaves = timer_slaves.elapsed();
	all_package.push_back(slave_cut_package);
	build_cut_full(_master, all_package, _problem_to_id, _trace, _slave_cut_id, _all_cuts_storage, _dynamic_aggregate_cuts, _data, _options);
	if (_options.BASIS) {
		SimplexBasisPackage slave_basis_package;
		AllBasisPackage all_basis_package;
		get_slave_basis(slave_basis_package, _map_slaves);
		all_basis_package.push_back(slave_basis_package);
		sort_basis(all_basis_package, _problem_to_id, _basis, _data);
	}
}

/*!
*  \brief Run Benders algorithm
*
*  Method to run Benders algorithm
*/
void Benders::run() {

	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	init(_data);
	_data.nrandom = _options.RAND_AGGREGATION;
	while (!_data.stop) {
		Timer timer_master;
		++_data.it;
		LOG_INFO_AND_COUT("ITERATION " + std::to_string(_data.it) + " :");
		LOG_INFO_AND_COUT("\tSolving master...");
		get_master_value(_master, _data, _options);

		LOG_INFO_AND_COUT("\tmaster solved in " + std::to_string(_data.timer_master) + ".");

		investment_candidates_log(_data);

		if (_options.ACTIVECUTS) {
			update_active_cuts(_master, _active_cuts, _slave_cut_id, _data.it);
		}

		_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));

		LOG_INFO_AND_COUT("\tBuilding cuts...");
		build_cut();
		LOG_INFO_AND_COUT("\tCuts built.");

		update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0, _data.best_it, _data.it);
		solution_log(_data);

		update_trace(_trace, _data);

		_data.timer_master = timer_master.elapsed();
		_data.stop = stopping_criterion(_data, _options);
	}

	if (_options.TRACE) {
		print_csv(_trace, _problem_to_id, _data, _options);
	}
	if (_options.ACTIVECUTS) {
		print_active_cut(_active_cuts,_options);
	}
}
