#pragma once


#include "Worker.h"
#include "SlaveCut.h"
#include "SimplexBasis.h"


/*!
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a slave problem
*/
class WorkerSlave;
typedef std::shared_ptr<WorkerSlave> WorkerSlavePtr;
typedef std::vector<WorkerSlavePtr> WorkerSlaves;
typedef std::map<std::string, WorkerSlavePtr> SlavesMapPtr;


class WorkerSlave : public Worker {
public:

	WorkerSlave();
	WorkerSlave(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options);
	virtual ~WorkerSlave();

public:

	void fix_to(Point const & x0);

	void get_subgradient(Point & s);

	SimplexBasis get_basis();

};


