
#include <algorithm>
#include <random>

#include "glog/logging.h"

#include "BendersMPI.h"

#define __DEBUG_BENDERS_MPI__ 0

BendersMpi::~BendersMpi() {

}

BendersMpi::BendersMpi(BendersOptions const & options, Logger &logger, mpi::environment & env, mpi::communicator & world):
BendersBase(options, logger), _exceptionRaised(false), _env(env), _world(world) {

}

/*!
*  \brief Method to load each problem in a thread
*
*  The initialization of each problem is done sequentially
*
*  \param problem_list : map linking each problem name to its variables and their id
*
*/

void BendersMpi::load(CouplingMap const & problem_list) {
	StrVector names;
	_data.nslaves = -1;
	std::vector<CouplingMap::const_iterator> real_problem_list;
	if (!problem_list.empty()) {
		if (_world.rank() == 0) {
			_data.nslaves = _options.SLAVE_NUMBER;
			if (_data.nslaves < 0) {
				_data.nslaves = problem_list.size() - 1;
			}
			std::string const & master_name(_options.MASTER_NAME);
			auto const it_master(problem_list.find(master_name));
			if (it_master == problem_list.end()) {
				std::cout << "UNABLE TO FIND " << master_name << std::endl;
				std::exit(1);
			}
			// real problem list taking into account SLAVE_NUMBER

			real_problem_list.resize(_data.nslaves, problem_list.end());

			CouplingMap::const_iterator it(problem_list.begin());
			for (int i(0); i < _data.nslaves; ++it) {
				if (it != it_master) {
					real_problem_list[i] = it;
					_problem_to_id[it->first] = i;
					++i;
				}
			}
			_master.reset(new WorkerMaster(it_master->second, _options.get_master_path(), _options, _data.nslaves));
			LOG(INFO) << "nrealslaves is " << _data.nslaves << std::endl;
		}
		mpi::broadcast(_world, _data.nslaves, 0);
		int current_worker(1);
		for (int islave(0); islave < _data.nslaves; ++islave, ++current_worker) {
			if (current_worker >= _world.size()) {
				current_worker = 1;
			}
			if (_world.rank() == 0) {
				CouplingMap::value_type kvp(*real_problem_list[islave]);
				_world.send(current_worker, islave, kvp);
			}
			else if (_world.rank() == current_worker) {
				CouplingMap::value_type kvp;
				_world.recv(0, islave, kvp);
				_map_slaves[kvp.first] = WorkerSlavePtr(new WorkerSlave(kvp.second, _options.get_slave_path(kvp.first), _options.slave_weight(_data.nslaves, kvp.first), _options));
				_slaves.push_back(kvp.first);
			}
		}
	}
}

/*!
*  \brief Update the value of options RAND_AGGREGATION according to the number of slaves on each thread
*
*	Update the value of options RAND_AGGREGATION according to the number of slaves on each thread
*
*  \param options : set of Benders options
*
*  \param options : set of Benders data
*
*  \param _env : environment variable for mpi communication
*
*  \param _world : communicator variable for mpi communication
*/
void BendersMpi::update_random_option() {
	int const n_thread(_world.size() - 1);
	int const n_max_slave(_data.nslaves % n_thread);
	int const n_sup_rand(_options.RAND_AGGREGATION % n_thread);
	int const n_slave_by_thread(_data.nslaves / n_thread);
	int const n_rand_slave(_options.RAND_AGGREGATION / n_thread);
	int const n_add_rand(n_thread * !(n_slave_by_thread == n_rand_slave) + n_max_slave* (n_slave_by_thread == n_rand_slave));
	if (_options.RAND_AGGREGATION && _world.rank() != 0) {
		_data.nrandom = n_rand_slave;
	}
	if (n_sup_rand) {
		std::set<int> set_rand_slave;
		if (_world.rank() == 0) {
			for (int i(0); i < n_sup_rand;) {
				if (set_rand_slave.insert(std::rand() % n_add_rand + 1).second) {
					i++;
				}
			}
		}
		boost::mpi::broadcast(_world, set_rand_slave, 0);
		if (set_rand_slave.find(_world.rank()) != set_rand_slave.end()) {
			_data.nrandom++;
		}
	}
}

/*!
*  \brief Solve, get and send solution of the Master Problem to every thread
*
*  \param _env : environment variable for mpi communication
*
*  \param _world : communicator variable for mpi communication
*/
void BendersMpi::step_1_solve_master() {

    int success = 1;
    try {
        do_solve_master_create_trace_and_update_cuts(_world.rank());
    }catch(std::exception& ex){
        success = 0;
        write_exception_message(ex);
    }
    check_if_some_proc_had_a_failure(success);
    broadcast_the_master_problem();
}

void BendersMpi::do_solve_master_create_trace_and_update_cuts(int rank) {
    if (rank == 0){
        solve_master_and_create_trace();
        if (_options.ACTIVECUTS) {
            update_active_cuts();
        }
    }
}

void BendersMpi::broadcast_the_master_problem() {
    if(!_exceptionRaised){
        mpi::broadcast(_world, _data.x0, 0);
        if (_options.RAND_AGGREGATION) {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(_slaves.begin(), _slaves.end(), g);
        }
        _world.barrier();
    }
}



void BendersMpi::solve_master_and_create_trace() {
    _logger->log_at_initialization(bendersDataToLogData(_data));
    _logger->display_message("\tSolving master...");
    get_master_value();
    _logger->log_master_solving_duration(_data.timer_master);
    _logger->log_iteration_candidates(bendersDataToLogData(_data));

    _trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
}

/*!
*  \brief Get cut information from each Slave and add it to the Master problem
*
*	Get cut information of every Slave Problem in each thread and send it to thread 0 to build new Master's cuts
*
*/
void BendersMpi::step_2_build_cuts() {
    solve_slaves_and_build_cuts();
    broadcast_rand_aggregation();
}

void BendersMpi::broadcast_rand_aggregation() {
    if(!_exceptionRaised) {
        mpi::broadcast(_world, _options.RAND_AGGREGATION, 0);
        _world.barrier();
    }
}

void BendersMpi::solve_slaves_and_build_cuts() {
    int success = 1;
    SlaveCutPackage slave_cut_package;
    Timer timer_slaves;
    try {
        if (_world.rank() != 0) {
            slave_cut_package = get_slave_package();
        }
    }catch(std::exception& ex){
        success = 0;
        write_exception_message(ex);
    }
    check_if_some_proc_had_a_failure(success);
    gather_slave_cut_package_and_build_cuts(slave_cut_package, timer_slaves);
}

void BendersMpi::gather_slave_cut_package_and_build_cuts(const SlaveCutPackage& slave_cut_package, const Timer& timer_slaves)
{
    if (!_exceptionRaised) {
        if (_world.rank() != 0){
            mpi::gather(_world, slave_cut_package, 0);
        }else{
            AllCutPackage all_package;
            mpi::gather(_world, slave_cut_package, all_package, 0);
            _data.timer_slaves = timer_slaves.elapsed();
            master_build_cuts(all_package);
        }
    }
}

SlaveCutPackage BendersMpi::get_slave_package()  {
    SlaveCutPackage slave_cut_package;
    if (_options.RAND_AGGREGATION) {
        get_random_slave_cut(slave_cut_package);
    }
    else {
        get_slave_cut(slave_cut_package);
    }
    return slave_cut_package;
}

void BendersMpi::master_build_cuts(AllCutPackage all_package) {

    _data.slave_cost = 0;
    for (auto const& pack : all_package) {
        for (auto& dataVal : pack) {
            _data.slave_cost += dataVal.second.first.second[SLAVE_COST];
        }
    }
    all_package.erase(all_package.begin());

    _logger->display_message("\tSolving subproblems...");

    build_cut_full(all_package);
    _logger->log_subproblems_solving_duration(_data.timer_slaves);
}

/*!
*  \brief Gather, store and sort all slaves basis in a set
*
*  \param _env : environment variable for mpi communication
*
*  \param _world : communicator variable for mpi communication
*/

void BendersMpi::check_if_some_proc_had_a_failure(int success) {
    int global_success;
    mpi::all_reduce(_world, success, global_success, mpi::bitwise_and<int>());
    if (global_success == 0){
        _exceptionRaised = true;
    }
}

void BendersMpi::write_exception_message(const std::exception &ex) {
    std::string error = "Exception raised : " + std::string(ex.what());
    LOG(WARNING) << error << std::endl;
    if(_logger){
        _logger->display_message(error);
    }
}

void BendersMpi::step_3_gather_slaves_basis() {

	SimplexBasisPackage slave_basis_package;
	if (_world.rank() == 0) {
		AllBasisPackage all_basis_package;
		mpi::gather(_world, slave_basis_package, all_basis_package, 0);
		all_basis_package.erase(all_basis_package.begin());
		sort_basis(all_basis_package);
	}
	else {
		get_slave_basis(slave_basis_package);
		mpi::gather(_world, slave_basis_package, 0);
	}
	_world.barrier();
}


void BendersMpi::step_4_update_best_solution(int rank, const Timer& timer_master, const Timer& benders_timer){
    if (rank == 0) {
        update_best_ub();
        _logger->log_at_iteration_end(bendersDataToLogData(_data));

        update_trace();

		_data.elapsed_time = benders_timer.elapsed();
        _data.timer_master = timer_master.elapsed();
        _data.stop = stopping_criterion();
    }
}

/*!
*  \brief Method to free the memory used by each problem
*/
void BendersMpi::free() {
	if (_world.rank() == 0)
		_master->free();
	else {
		for (auto & ptr : _map_slaves)
			ptr.second->free();
	}
	_world.barrier();
}

/*!
*  \brief Run Benders algorithm in parallel
*
*  Method to run Benders algorithm in parallel
*
*/
void BendersMpi::run() {
	if (_world.rank() == 0) {
		for (auto const & kvp : _problem_to_id) {
			_all_cuts_storage[kvp.first] = SlaveCutStorage();
		}
	}
	init_data();
	_world.barrier();

	Timer benders_timer;
	while (!_data.stop) {
		Timer timer_master;
		update_random_option();
		++_data.it;
		_data.deletedcut = 0;

		/*Solve Master problem, get optimal value and cost and send it to Slaves*/
		step_1_solve_master();

		/*Gather cut from each slave in master thread and add them to Master problem*/
		if (!_exceptionRaised) {
            step_2_build_cuts();
        }

		if (_options.BASIS && !_exceptionRaised) {
            step_3_gather_slaves_basis();
		}

        if (!_exceptionRaised) {
            step_4_update_best_solution(_world.rank(), timer_master, benders_timer);
        }
        _data.stop |= _exceptionRaised;

		broadcast(_world, _data.stop, 0);
	}

	if (_world.rank() == 0) {
		if (_options.TRACE) {
			print_csv();
		}
		if (_options.ACTIVECUTS) {
			print_active_cut();
		}
	}
}
