#include "catch2.hpp"
#include "BendersFunctions.h"

TEST_CASE("Update best ub", "[Bd-functions]") {
	BendersData data;
	init(data);

	Point bestx;
	Point x0;
	x0["1"] = 1.0;
	x0["2"] = 2.0;
	x0["3"] = 3.0;
	
	double bestub = 1500.0;
	double ub = 120.0;
	int it = 4;
	int bestit;

	update_best_ub(bestub, ub, bestx, x0, bestit, it);
	REQUIRE(bestx == x0);
	REQUIRE(bestit == it);
	REQUIRE(bestub == ub);
}