#include "ProblemGenerationExeOptions.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

class ProblemGenerationExeOptionsTest : public ::testing::Test {
 protected:
  ProblemGenerationExeOptions problem_generation_options_parser_;
};

TEST_F(ProblemGenerationExeOptionsTest, WithOutOutputOption) {
  char argv = 'c';
  char* pargv = &argv;
  char** ppargv = &pargv;
  try {
    problem_generation_options_parser_.Parse(1, ppargv);
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--output' is required but missing"));
  }
}

