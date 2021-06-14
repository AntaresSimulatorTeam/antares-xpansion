#pragma once

#include "BendersOptions.h"
#include "common_mpi.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersFunctions.h"

#include "benders_sequential_core/ILogger.h"

/*!
* \class BendersMpi
* \brief Class use run the benders algorithm in parallel 
*/
class BendersMpi {

public:

	virtual ~BendersMpi();
	BendersMpi(mpi::environment & env, mpi::communicator & world, BendersOptions const & options, Logger &logger);

	void load(CouplingMap const & problem_list, mpi::environment & env, mpi::communicator & world);
	
	WorkerMasterPtr _master;
	SlavesMapPtr _map_slaves;
	
	Str2Int _problem_to_id;
	BendersData _data;
	BendersOptions _options;
	StrVector _slaves;

	AllCutStorage _all_cuts_storage;
	BendersTrace _trace;
	DynamicAggregateCuts _dynamic_aggregate_cuts;

	SimplexBasisStorage _basis;
	SlaveCutId _slave_cut_id;
	ActiveCutStorage _active_cuts;

	void free(mpi::environment & env, mpi::communicator & world);
	bool step_1_solve_master(mpi::environment & env, mpi::communicator & world);
    bool step_2_build_cuts(mpi::environment & env, mpi::communicator & world);
    bool step_3_gather_slaves_basis(mpi::environment & env, mpi::communicator & world);
	void update_random_option(mpi::environment & env, mpi::communicator & world, BendersOptions const & options, BendersData & data);
	void run(mpi::environment & env, mpi::communicator & world);

private:

    Logger _logger;

    void master_build_cuts(AllCutPackage all_package);
    SlaveCutPackage get_slave_package();
};
