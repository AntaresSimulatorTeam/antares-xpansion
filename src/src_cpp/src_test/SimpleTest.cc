#include "gtest/gtest.h"



TEST(Simple, print)
{

	ASSERT_EQ(2, 2);
	std::cout << "true.\n";
}