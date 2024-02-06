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

class FullRunOptionsParserTest
    : public ::testing::TestWithParam<std::vector<std::string>> {
 protected:
  FullRunOptionsParser full_run_options_options_parser_;
};
class FullRunOptionsParserTestFullPath : public FullRunOptionsParserTest {};

class FullRunOptionsParserTestParameterizedMethod_output
    : public ::testing::TestWithParam<std::pair<std::string, BENDERSMETHOD>> {
 protected:
  FullRunOptionsParser full_run_options_options_parser_;
};

auto params() {
  return ::testing::ValuesIn(std::vector<std::vector<std::string>>{
      {"full_run.exe", "--archive", "something"},
      {"full_run.exe", "--output", "something"},
  });
}
TEST_P(FullRunOptionsParserTest, ThatBendersOptionFileIsRequired) {
  auto params = GetParam();
  std::vector<const char*> pargs;
  std::ranges::transform(params, std::back_inserter(pargs),
                         [](const std::string& s) { return s.data(); });
  try {
    full_run_options_options_parser_.Parse(params.size(), pargs.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(
        e.what(),
        std::string("the option '--benders_options' is required but missing"));
  }
}

TEST_P(FullRunOptionsParserTest, ThatMethodOptionIsRequired) {
  auto params = GetParam();
  std::vector<const char*> pargs;
  std::ranges::transform(params, std::back_inserter(pargs),
                         [](const std::string& s) { return s.data(); });
  const char argv5[] = "--benders_options";
  const char argv6[] = "something";
  pargs.push_back(argv5);
  pargs.push_back(argv6);
  try {
    full_run_options_options_parser_.Parse(pargs.size(), pargs.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--method' is required but missing"));
  }
}

TEST_P(FullRunOptionsParserTest, ThatSolutionOptionIsRequired) {
  auto params = GetParam();
  std::vector<const char*> pargs;
  std::ranges::transform(params, std::back_inserter(pargs),
                         [](const std::string& s) { return s.data(); });
  const char argv5[] = "--benders_options";
  const char argv6[] = "something";
  pargs.push_back(argv5);
  pargs.push_back(argv6);
  const char argv7[] = "-m";
  const char argv8[] = "mpibenders";
  pargs.push_back(argv7);
  pargs.push_back(argv8);
  try {
    full_run_options_options_parser_.Parse(pargs.size(), pargs.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--solution' is required but missing"));
  }
}
INSTANTIATE_TEST_SUITE_P(args, FullRunOptionsParserTest, params());

TEST_P(FullRunOptionsParserTestFullPath, OptionsParsing) {
  auto params = GetParam();
  std::vector<const char*> pargs;
  std::ranges::transform(params, std::back_inserter(pargs),
                         [](const std::string& s) { return s.data(); });
  const char argv5[] = "--benders_options";
  const char argv6[] = "something";
  pargs.push_back(argv5);
  pargs.push_back(argv6);
  const char argv7[] = "-m";
  const char argv8[] = "benders";
  pargs.push_back(argv7);
  pargs.push_back(argv8);
  const char argv9[] = "-s";
  const char argv10[] = "/path/to/solution.json";
  pargs.push_back(argv9);
  pargs.push_back(argv10);
  full_run_options_options_parser_.Parse(pargs.size(), pargs.data());
  ASSERT_EQ(full_run_options_options_parser_.BendersOptionsFile(),
            std::filesystem::path(argv6));
  ASSERT_EQ(full_run_options_options_parser_.SolutionFile(),
            std::filesystem::path(argv10));
  ASSERT_EQ(full_run_options_options_parser_.Method(), BENDERSMETHOD::BENDERS);
}
auto full_path_params() {
  return ::testing::ValuesIn(std::vector<std::vector<std::string>>{
      {"full_run.exe", "--archive", "/path/to/output.zip"},
      {"full_run.exe", "--output", "/path/to/output"},
  });
}
INSTANTIATE_TEST_SUITE_P(args, FullRunOptionsParserTestFullPath,
                         full_path_params());

TEST_P(FullRunOptionsParserTestParameterizedMethod_output, AllowedMethods) {
  const char argv0[] = "full_run.exe";
  const char argv3[] = "--output";
  const char argv4[] = "/path/to/output";
  const char argv5[] = "--benders_options";
  const char argv6[] = "options.json";
  const char argv7[] = "-m";
  const char argv8[] = "-s";
  const char argv9[] = "/path/to/solution.json";
  auto [method_str, expected_method_enum] = GetParam();
  std::vector<const char*> ppargv = {
      argv0, argv3, argv4, argv5,
                                     argv6, argv7, method_str.c_str(),
                                     argv8, argv9};
  full_run_options_options_parser_.Parse(ppargv.size(), ppargv.data());
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
INSTANTIATE_TEST_SUITE_P(Method,
                         FullRunOptionsParserTestParameterizedMethod_output,
                         GetData());