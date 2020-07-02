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
	PointPtr _min_invest;
	PointPtr _max_invest;
	std::map<std::string,SlaveCutDataPtr> _cut_trace;

	double _invest_cost;
	double _operational_cost;

	Point get_point();
	Point get_min_invest();
	Point get_max_invest();
};

typedef std::shared_ptr<WorkerMasterData> WorkerMasterDataPtr;
typedef std::vector<WorkerMasterDataPtr> BendersTrace;


