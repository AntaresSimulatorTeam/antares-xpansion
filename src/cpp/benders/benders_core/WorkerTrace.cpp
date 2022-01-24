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

LogData defineLogDataFromBendersDataAndTrace(const BendersData& data, const BendersTrace& trace){

    LogData result;

    result.it = data.it;
    result.best_it = data.best_it;
    result.lb = data.lb;
    result.best_ub = data.best_ub;
    result.x0 = data.bestx;
    size_t bestItIndex_l = data.best_it - 1;

    if (bestItIndex_l >= 0 && bestItIndex_l < trace.size())
    {
        const WorkerMasterDataPtr& bestItTrace = trace[bestItIndex_l];
        result.slave_cost = bestItTrace->_operational_cost;
        result.invest_cost = bestItTrace->_invest_cost;
        result.min_invest = bestItTrace->get_min_invest();
        result.max_invest = bestItTrace->get_max_invest();
    }

    return result;
}


