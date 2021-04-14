#include "catch2.hpp"
#include "WorkerTrace.h"

TEST_CASE("WorkerTrace", "[wrk-trace]") {

	WorkerMasterDataPtr trace(new WorkerMasterData);

	Point x;
	x["A"] = 1.0;
	x["B"] = 1.5;
	x["C"] = 2.0;
	PointPtr ptrx(new Point(x));

	trace->_x0 = ptrx;
	REQUIRE(trace->get_point() == x);

	Point mx;
	mx["mA"] = 1.0;
	mx["mB"] = 1.5;
	mx["mC"] = 2.0;
	PointPtr ptrMin(new Point(mx));
	trace->_min_invest = ptrMin;
	REQUIRE(trace->get_min_invest() == mx);

	Point Mx;
	Mx["MA"] = 1.0;
	Mx["MB"] = 1.5;
	Mx["MC"] = 2.0;
	PointPtr ptrMax(new Point(Mx));
	trace->_max_invest = ptrMax;
	REQUIRE(trace->get_max_invest() == Mx);
}