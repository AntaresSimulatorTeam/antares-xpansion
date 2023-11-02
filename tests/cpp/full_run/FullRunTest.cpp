#include <utility>

#include "FullRunOptionsParser.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

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

TEST_P(FullRunOptionsParserTest, ThatSolutionOptionIsRequired) {
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
  const char argv7[] = "-s";
  const char argv8[] = "/path/to/solution.json";
  pargs.push_back(argv7);
  pargs.push_back(argv8);

  full_run_options_options_parser_.Parse(pargs.size(), pargs.data());
  ASSERT_EQ(full_run_options_options_parser_.BendersOptionsFile(),
            std::filesystem::path(argv6));
  ASSERT_EQ(full_run_options_options_parser_.SolutionFile(),
            std::filesystem::path(argv8));
}

auto full_path_params() {
  return ::testing::ValuesIn(std::vector<std::vector<std::string>>{
      {"full_run.exe", "--archive", "/path/to/output.zip"},
      {"full_run.exe", "--output", "/path/to/output"},
  });
}
INSTANTIATE_TEST_SUITE_P(args, FullRunOptionsParserTestFullPath,
                         full_path_params());
