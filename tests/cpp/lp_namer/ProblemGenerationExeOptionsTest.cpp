#include "ProblemGenerationExeOptions.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

class ProblemGenerationExeOptionsTest : public ::testing::Test {
 protected:
  ProblemGenerationExeOptions problem_generation_options_parser_;
};

TEST_F(ProblemGenerationExeOptionsTest, WithEmptyArgs) {
  problem_generation_options_parser_.Parse(1, nullptr);
}

