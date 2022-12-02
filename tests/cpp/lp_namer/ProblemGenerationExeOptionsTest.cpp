#include "ProblemGenerationExeOptions.h"
#include "gtest/gtest.h"

namespace po = boost::program_options;

class ProblemGenerationExeOptionsTest : public ::testing::Test {
 protected:
  ProblemGenerationExeOptions problem_generation_options_parser_;
};

TEST_F(ProblemGenerationExeOptionsTest, WithOutArchiveOption) {
  char argv = 'c';
  char* pargv = &argv;
  char** ppargv = &pargv;
  try {
    problem_generation_options_parser_.Parse(1, ppargv);
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--archive' is required but missing"));
  }
}
TEST_F(ProblemGenerationExeOptionsTest, WithOutOutputOption) {
  const char argv0[] = "lp_namer.exe";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  std::vector<const char*> ppargv = {argv0, argv1, argv2};
  try {
    problem_generation_options_parser_.Parse(3, ppargv.data());
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--output' is required but missing"));
  }
}

TEST_F(ProblemGenerationExeOptionsTest, MasterFormulationDefaultValue) {
  const char argv0[] = "lp.exe ";
  const char argv1[] = "--archive";
  const char argv2[] = "something";
  const char argv3[] = "--output";
  const char argv4[] = "something";
  std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3, argv4};
  problem_generation_options_parser_.Parse(5, ppargv.data());
  ASSERT_EQ(problem_generation_options_parser_.MasterFormulation(),
            std::string("relaxed"));
}
// TEST_F(ProblemGenerationExeOptionsTest,
// AdditionalConstraintsFilenameIsNotEmptyIfGiven) {
//   const char argv0[] = "lp.exe ";
//   const char argv1[] = "--output";
//   const char argv2[] = "something";
//   const char argv3[] = "-e";
//   std::vector<const char*> ppargv = {argv0, argv1, argv2, argv3};
//   try {
//     problem_generation_options_parser_.Parse(4, ppargv.data());
//   } catch (const std::exception& e) {
//     EXPECT_EQ(e.what(),
//               std::string("the required argument for option
//               '--exclusion-files' is missing"));
//   }
// }
