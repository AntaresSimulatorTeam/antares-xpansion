#include <utility>

#include "FullRunOptionsParser.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;
// struct CommandLine {
//   std::string exe_name;
//   std::string output = "--output";
//   std::string output_value;
//   std::string benders_options = "--benders_options";
//   std::string benders_options_value;
//   std::string method = "--method";
//   std::string method_value;
//   std::string solution = "--solution";
//   std::string solution_value;
//   // std::string ToString() const {
//   //   return exe_name + " " + output + " " + output_value + " " +
//   //          benders_options + " " + benders_options_value + " " + method +
//   " "
//   //          + method_value + " " + solution + " " + solution_value;
//   // }
// };

class FullRunOptionsParserTest : public ::testing::Test {
 protected:
  FullRunOptionsParser full_run_options_options_parser_;
};
class FullRunOptionsParserTestParameterizedMethod
    : public ::testing::TestWithParam<std::pair<std::string, BENDERSMETHOD>> {
 protected:
  FullRunOptionsParser full_run_options_options_parser_;
};

TEST_F(FullRunOptionsParserTest, ThatBendersOptionFileIsRequired) {
  const char argv0[] = "full_run.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  const char argv3[] = "--output";
  const char argv4[] = "something";
  std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3, argv4};
  try {
    full_run_options_options_parser_.Parse(5, ppargv.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(
        e.what(),
        std::string("the option '--benders_options' is required but missing"));
  }
}
TEST_F(FullRunOptionsParserTest, ThatMethodOptionIsRequired) {
  const char argv0[] = "full_run.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  const char argv3[] = "--output";
  const char argv4[] = "something";
  const char argv5[] = "--benders_options";
  const char argv6[] = "something";
  std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3,
                                     argv4, argv5, argv6};
  try {
    full_run_options_options_parser_.Parse(7, ppargv.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--method' is required but missing"));
  }
}
TEST_F(FullRunOptionsParserTest, ThatSolutionOptionIsRequired) {
  const char argv0[] = "full_run.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  const char argv3[] = "--output";
  const char argv4[] = "something";
  const char argv5[] = "--benders_options";
  const char argv6[] = "something";
  const char argv7[] = "-m";
  const char argv8[] = "mpibenders";
  std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3, argv4,
                                     argv5, argv6, argv7, argv8};
  try {
    full_run_options_options_parser_.Parse(9, ppargv.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--solution' is required but missing"));
  }
}
TEST_F(FullRunOptionsParserTest, OptionsParsing) {
  const char argv0[] = "full_run.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "/path/to/output.zip";
  const char argv3[] = "--output";
  const char argv4[] = "/path/to/output";
  const char argv5[] = "--benders_options";
  const char argv6[] = "options.json";
  const char argv7[] = "-m";
  const char argv8[] = "benders";
  const char argv9[] = "-s";
  const char argv10[] = "/path/to/solution.json";
  std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3, argv4, argv5,
                                     argv6, argv7, argv8, argv9, argv10};
  full_run_options_options_parser_.Parse(11, ppargv.data());
  ASSERT_EQ(full_run_options_options_parser_.ArchivePath(),
            std::filesystem::path(argv2));
  ASSERT_EQ(full_run_options_options_parser_.BendersOptionsFile(),
            std::filesystem::path(argv6));
  ASSERT_EQ(full_run_options_options_parser_.SolutionFile(),
            std::filesystem::path(argv10));
  ASSERT_EQ(full_run_options_options_parser_.Method(), BENDERSMETHOD::BENDERS);
}
TEST_P(FullRunOptionsParserTestParameterizedMethod, AllowedMethods) {
  const char argv0[] = "full_run.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "/path/to/output.zip";
  const char argv3[] = "--output";
  const char argv4[] = "/path/to/output";
  const char argv5[] = "--benders_options";
  const char argv6[] = "options.json";
  const char argv7[] = "-m";
  const char argv8[] = "-s";
  const char argv9[] = "/path/to/solution.json";
  auto [method_str, expected_method_enum] = GetParam();
  std::vector<const char*> ppargv = {argv0, argv1, argv2,
                                     argv3, argv4, argv5,
                                     argv6, argv7, method_str.c_str(),
                                     argv8, argv9};
  full_run_options_options_parser_.Parse(11, ppargv.data());
  ASSERT_EQ(full_run_options_options_parser_.ArchivePath(),
            std::filesystem::path(argv2));
  ASSERT_EQ(full_run_options_options_parser_.BendersOptionsFile(),
            std::filesystem::path(argv6));
  ASSERT_EQ(full_run_options_options_parser_.SolutionFile(),
            std::filesystem::path(argv9));
  ASSERT_EQ(full_run_options_options_parser_.Method(), expected_method_enum);
}
auto GetData() {
  return ::testing::ValuesIn(std::vector<std::pair<std::string, BENDERSMETHOD>>{
      {"benders", BENDERSMETHOD::BENDERS},
      {"benders_by_batch", BENDERSMETHOD::BENDERSBYBATCH},
      {"mergeMPS", BENDERSMETHOD::MERGEMPS}});
}
INSTANTIATE_TEST_CASE_P(Method, FullRunOptionsParserTestParameterizedMethod,
                        GetData());