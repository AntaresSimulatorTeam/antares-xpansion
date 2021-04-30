#include "SlaveCut.h"
#include "catch2.hpp"

TEST_CASE("Test SlaveCutDataHandler: setters and getters", "[cut]") {

	SlaveCutDataPtr slave_cut_data(new SlaveCutData);
	SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

	REQUIRE(handler->get_int().size() == SlaveCutInt::MAXINTEGER);
	REQUIRE(handler->get_dbl().size() == SlaveCutDbl::MAXDBL);
	REQUIRE(handler->get_str().size() == SlaveCutStr::MAXSTR);

	Point s;
	s["A"] = 1.0;
	s["B"] = 1.5;
	s["C"] = 2.0;

	handler->get_subgradient() = s;
	REQUIRE(handler->get_subgradient() == s);

	handler->get_int(SlaveCutInt::SIMPLEXITER) = 5;
	handler->get_int(SlaveCutInt::LPSTATUS) = 2;
	REQUIRE(handler->get_int(SlaveCutInt::SIMPLEXITER) == 5);
	REQUIRE(handler->get_int(SlaveCutInt::LPSTATUS) == 2);

	handler->get_dbl(SlaveCutDbl::SLAVE_COST) = 5.0;
	handler->get_dbl(SlaveCutDbl::ALPHA_I) = 2.7;
	handler->get_dbl(SlaveCutDbl::SLAVE_TIMER) = 0.321;
	REQUIRE(handler->get_dbl(SlaveCutDbl::SLAVE_COST) == 5.0);
	REQUIRE(handler->get_dbl(SlaveCutDbl::ALPHA_I) == 2.7);
	REQUIRE(handler->get_dbl(SlaveCutDbl::SLAVE_TIMER) == 0.321);

	IntVector intVec = { 9, 87 };
	handler->get_int() = intVec;
	REQUIRE(handler->get_int() == intVec);

	const DblVector dblVec = { 0.32, 3.22, 8.0 };
	handler->get_dbl() = dblVec;
	REQUIRE(handler->get_dbl() == dblVec);
}
