#pragma once


#include "Worker.h"
#include "SlaveCut.h"

/*!
* \class WorkerMasterData
* \brief Class use to store trace information during the algorithm run
*/
class WorkerMasterData {
public:

	double _lb;
	double _ub;
	double _bestub;
	int _deleted_cut;
	int _nbasis;
	double _time;
	PointPtr _x0;
	std::map<std::string,SlaveCutDataPtr> _cut_trace;

	Point get_point();
};

typedef std::shared_ptr<WorkerMasterData> WorkerMasterDataPtr;
typedef std::vector<WorkerMasterDataPtr> BendersTrace;


