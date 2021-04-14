#include "WorkerTrace.h"

/*!
* \brief Get point
*/
Point WorkerMasterData::get_point() {
	return *_x0;
}

Point WorkerMasterData::get_min_invest() {
	return *_min_invest;
}

Point WorkerMasterData::get_max_invest() {
	return *_max_invest;
}


