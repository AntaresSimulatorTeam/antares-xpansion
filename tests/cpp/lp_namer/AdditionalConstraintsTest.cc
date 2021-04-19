#include "gtest/gtest.h"

#include <sstream>
#include <fstream>

#include "AdditionalConstraints.h"

class AdditionalConstraintsTest : public ::testing::Test
{
protected:
	static void SetUpTestCase()
	{
		// called before 1st test
		std::string content_l = "\
[variables]\n\
grid = bin_grid\n\
grid2 = bin_grid2\n\
ccgt1 = bin_ccgt1\n\
ccgt2 = bin_ccgt2\n\
ccgt3 = bin_ccgt3\n\
[binary_constraint_3]\n\
name = additional_c3\n\
bin_grid = 1\n\
bin_grid2 = 1\n\
bin_ccgt1 = 1\n\
bin_ccgt2 = 1\n\
bin_ccgt3 = 1\n\
sign = less_or_equal\n\
rhs = 3\n\
[1]\n\
name = additional_c1\n\
grid = 1\n\
grid2 = 1\n\
sign = less_or_equal\n\
rhs = 3000\n\
[2]\n\
name = additional_c2\n\
ccgt1 = 1\n\
ccgt2 = 1\n\
ccgt3 = 1\n\
sign = equal\n\
rhs = 3000\n\
[10]\n\
name = my_ge_constraint     \n\
ccgt1 =    1\n\
ccgt2 =    1\n\
ccgt3 =   1\n\
grid     = 1\n\
grid2 =   1\n\
sign = greater_or_equal\n\
rhs = 200\
";

	// dummy additional contraints tmp file name
	std::ofstream file_l("temp_additional_constraints.ini");
	file_l << content_l;
	file_l.close();
	}

	static void TearDownTestCase()
	{
		// called after last test

		//delete the created tmp file
		std::remove("temp_additional_constraints.ini");
	}

	void SetUp()
	{
		// called before each test
	}

   void TearDown()
	{
		// called after each test
	}
};

TEST_F(AdditionalConstraintsTest, testCreation)
{
	AdditionalConstraints additionalConstraints_l("temp_additional_constraints.ini");

	ASSERT_EQ(additionalConstraints_l.getVariablesToBinarise().size(), 5);
	ASSERT_EQ(additionalConstraints_l.size(), 4);

	std::vector<std::string> expectedConstraintNames_l = {"additional_c1", "additional_c2", "additional_c3", "my_ge_constraint"};
	size_t cnt_l(0);
	for ( auto pairNameConstraint : additionalConstraints_l )
	{
		ASSERT_EQ(pairNameConstraint.first, expectedConstraintNames_l[cnt_l++]);
	}

	std::vector<double> expectedRHS_l = {3000, 3000, 3, 200};
	cnt_l = 0;
	for ( auto pairNameConstraint : additionalConstraints_l )
	{
		ASSERT_EQ(pairNameConstraint.second.getRHS(), expectedRHS_l[cnt_l++]);
	}

	std::vector<std::string> expectedSign_l = {"less_or_equal", "equal", "less_or_equal", "greater_or_equal"};
	cnt_l = 0;
	for ( auto pairNameConstraint : additionalConstraints_l )
	{
		ASSERT_EQ(pairNameConstraint.second.getSign(), expectedSign_l[cnt_l++]);
	}
}