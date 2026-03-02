#include <fstream>
#include <sstream>

#include "antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h"
#include "antares-xpansion/lpnamer/problem_modifier/LauncherHelpers.h"
#include "LoggerBuilder.h"
#include "gtest/gtest.h"

class AdditionalConstraintsTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    // called before 1st test
    std::string content_l =
        "\
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

  static void TearDownTestCase() {
    // called after last test

    // delete the created tmp file
    std::remove("temp_additional_constraints.ini");
  }

  void SetUp() {
    // called before each test
  }

  void TearDown() {
    // called after each test
  }
};

TEST_F(AdditionalConstraintsTest, testCreation) {
  auto logger = emptyLogger();
  AdditionalConstraints additionalConstraints_l(
      "temp_additional_constraints.ini", logger);

  ASSERT_EQ(additionalConstraints_l.getVariablesToBinarise().size(), 5);
  ASSERT_EQ(additionalConstraints_l.size(), 4);

  std::vector<std::string> expectedConstraintNames_l = {
      "additional_c1", "additional_c2", "additional_c3", "my_ge_constraint"};
  std::vector<double> expectedRHS_l = {3000, 3000, 3, 200};
  std::vector<std::string> expectedSign_l = {
      "less_or_equal", "equal", "less_or_equal", "greater_or_equal"};
  std::vector<char> expected_rtype = {'L', 'E', 'L', 'G'};

  size_t cnt_l(0);
  for (auto const& [name, constraint] : additionalConstraints_l) {
    ASSERT_EQ(name, expectedConstraintNames_l[cnt_l]);
    ASSERT_EQ(constraint.getRHS(), expectedRHS_l[cnt_l]);
    ASSERT_EQ(constraint.getSign(), expectedSign_l[cnt_l]);
    ASSERT_EQ(getConstraintSenseSymbol(constraint, logger),
              expected_rtype[cnt_l]);
    cnt_l++;
  }
}