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
