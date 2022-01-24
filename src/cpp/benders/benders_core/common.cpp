#include "common.h"


/*!
*  \brief Return the distance between two point using 2-norm
*
*  \param x0 : first point
*
*  \param x1 : second point
*/
double norm_point(Point const & x0, Point const & x1) {
	double result(0);
	for (auto & kvp : x0) {
		result += (x0.find(kvp.first)->second - x1.find(kvp.first)->second)*(x0.find(kvp.first)->second - x1.find(kvp.first)->second);
	}
	result = std::sqrt(result);
	return result;
}

LogData bendersDataToLogData(const BendersData &data) {
    LogData result;
    result.lb = data.lb;
    result.best_ub = data.best_ub;
    result.it = data.it;
    result.best_it = data.best_it;
    result.slave_cost = data.slave_cost;
    result.invest_cost = data.invest_cost;
    result.x0 = data.x0;
    result.min_invest = data.min_invest;
    result.max_invest = data.max_invest;
    return result;
}
