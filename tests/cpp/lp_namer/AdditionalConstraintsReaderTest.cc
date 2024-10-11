#include <fstream>
#include <sstream>

#include "AdditionalConstraintsReader.h"
#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "gtest/gtest.h"

class AdditionalConstraintsReaderTest : public ::testing::Test {
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

TEST_F(AdditionalConstraintsReaderTest, testSections) {
  auto logger = emptyLogger();
  AdditionalConstraintsReader additionalConstraintsReader_l(
      "temp_additional_constraints.ini", logger);

  std::set<std::string> sections_l =
      additionalConstraintsReader_l.getSections();

  std::set<std::string> expectedSections_l = {"binary_constraint_3",
                                              "variables", "1", "2", "10"};
  ASSERT_EQ(expectedSections_l.size(), sections_l.size());

  auto expected_it = expectedSections_l.begin();
  for (auto sections_it = sections_l.begin(); sections_it != sections_l.end();
       ++sections_it) {
    ASSERT_EQ(*expected_it, *sections_it);
    ++expected_it;
  }
}

TEST_F(AdditionalConstraintsReaderTest, testVariables) {
  auto logger = emptyLogger();
  AdditionalConstraintsReader additionalConstraintsReader_l(
      "temp_additional_constraints.ini", logger);

  std::map<std::string, std::string> variablesSection_l =
      additionalConstraintsReader_l.getVariablesSection();

  std::map<std::string, std::string> expectedvars_l;
  expectedvars_l["grid"] = "bin_grid";
  expectedvars_l["grid2"] = "bin_grid2";
  expectedvars_l["ccgt1"] = "bin_ccgt1";
  expectedvars_l["ccgt2"] = "bin_ccgt2";
  expectedvars_l["ccgt3"] = "bin_ccgt3";

  ASSERT_EQ(expectedvars_l.size(), variablesSection_l.size());

  auto expected_it = expectedvars_l.begin();
  for (auto vars_it = variablesSection_l.begin();
       vars_it != variablesSection_l.end(); ++vars_it) {
    ASSERT_EQ(expected_it->first, vars_it->first);
    ASSERT_EQ(expected_it->second, vars_it->second);
    ++expected_it;
  }
}

TEST_F(AdditionalConstraintsReaderTest, testOneSection) {
  auto logger = emptyLogger();
  AdditionalConstraintsReader additionalConstraintsReader_l(
      "temp_additional_constraints.ini", logger);

  std::map<std::string, std::string> section_l =
      additionalConstraintsReader_l.getSection("10");

  std::map<std::string, std::string> expectedVals;
  expectedVals["name"] = "my_ge_constraint";
  expectedVals["ccgt1"] = "1";
  expectedVals["ccgt2"] = "1";
  expectedVals["ccgt3"] = "1";
  expectedVals["grid"] = "1";
  expectedVals["grid2"] = "1";
  expectedVals["sign"] = "greater_or_equal";
  expectedVals["rhs"] = "200";

  ASSERT_EQ(expectedVals.size(), section_l.size());

  auto expected_it = expectedVals.begin();
  for (auto vals_it = section_l.begin(); vals_it != section_l.end();
       ++vals_it) {
    ASSERT_EQ(expected_it->first, vals_it->first);
    ASSERT_EQ(expected_it->second, vals_it->second);
    ++expected_it;
  }
}