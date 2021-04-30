#include "SimplexBasis.h"
#include "catch2.hpp"

TEST_CASE("Test getters and setters Simplex Basis", "[basis]"){

	SimplexBasisPtr basis(new SimplexBasis);
	SimplexBasisHandler handler(basis);

	IntVector rows = { 0,2,4,6 };
	handler.get_row() = rows;
	REQUIRE(handler.get_row() == rows);

	IntVector cols = { 1, 3, 5, 7 };
	handler.get_col() = cols;
	REQUIRE(handler.get_col() == cols);
}