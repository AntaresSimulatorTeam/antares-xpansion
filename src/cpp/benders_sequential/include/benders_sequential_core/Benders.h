#pragma once

#include "WorkerMaster.h"
#include "WorkerTrace.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"
#include "SimplexBasis.h"
#include "ILogger.h"


/*!
* \class Benders
* \brief Class use run the benders algorithm in sequential
*/
class Benders {
public:
	explicit Benders(CouplingMap const &problem_list, BendersOptions const &options, Logger &logger);
    explicit Benders( Logger &logger);
	virtual ~Benders();

	WorkerMasterPtr _master;
	SlavesMapPtr _map_slaves;

	Str2Int _problem_to_id;
	BendersData _data;
	BendersOptions _options;
	StrVector _slaves;

	BendersTrace _trace;
	AllCutStorage _all_cuts_storage;
	DynamicAggregateCuts _dynamic_aggregate_cuts;

	SimplexBasisStorage _basis;
	SlaveCutId _slave_cut_id;
	ActiveCutStorage _active_cuts;

	void free();

	void build_cut();
	void run( CouplingMap const &problem_list, BendersOptions const &options);
    void run();
private:
    Logger _logger;

    void initialise_problems(const CouplingMap &problem_list);
};
