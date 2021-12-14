#include "Benders.h"

#include "solver_utils.h"

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

Benders::Benders(BendersOptions const &options, Logger &logger):BendersBase(options, logger) {

}

void Benders::initialise_problems(const CouplingMap &problem_list) {
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
                _map_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path(it->first), _options.slave_weight(
                        _data.nslaves, it->first), _options));
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
	if (_master)
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
	get_slave_cut(slave_cut_package);
	_data.slave_cost = 0;
	for (auto pairSlavenameSlavecutdata_l : slave_cut_package)
	{
		_data.slave_cost += pairSlavenameSlavecutdata_l.second.first.second[SLAVE_COST];
	}

	
	_data.timer_slaves = timer_slaves.elapsed();
	all_package.push_back(slave_cut_package);
	build_cut_full( all_package);
	if (_options.BASIS) {
		SimplexBasisPackage slave_basis_package;
		AllBasisPackage all_basis_package;
		get_slave_basis(slave_basis_package);
		all_basis_package.push_back(slave_basis_package);
		sort_basis(all_basis_package);
	}
}

/*!
*  \brief Run Benders algorithm
*
*  Method to run Benders algorithm
*/
void Benders::run( CouplingMap const &problem_list) {
    initialise_problems(problem_list);
    doRun();
}

void Benders::doRun(){
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	init_data();
	Timer benders_timer;
	while (!_data.stop) {
		Timer timer_master;
		++_data.it;

		_logger->log_at_initialization(bendersDataToLogData(_data));
		_logger->display_message("\tSolving master...");
		get_master_value();
		_logger->log_master_solving_duration( _data.timer_master);

		_logger->log_iteration_candidates(bendersDataToLogData(_data));

		if (_options.ACTIVECUTS) {
			update_active_cuts();
		}

		_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));

		_logger->display_message("\tSolving subproblems...");
		build_cut();
		_logger->log_subproblems_solving_duration(_data.timer_slaves);

		update_best_ub();

		_logger->log_at_iteration_end(bendersDataToLogData(_data));

		update_trace();

		_data.timer_master = timer_master.elapsed();
		_data.elapsed_time = benders_timer.elapsed();
		_data.stop = stopping_criterion();
	}

	if (_options.TRACE) {
		print_csv();
	}
	if (_options.ACTIVECUTS) {
		print_active_cut();
	}
}
