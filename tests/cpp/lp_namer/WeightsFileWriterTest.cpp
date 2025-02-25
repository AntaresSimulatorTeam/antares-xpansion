#include <fstream>
#include <memory>

#include "NOOPSolver.h"
#include "antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h"
#include "gtest/gtest.h"

class MockProblem : public Problem {
 public:
  MockProblem(int year) : Problem(std::make_shared<NOOPSolver>()) {
    Problem::mc_year = year;
  }
};

class WeightsFileWriterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const ::testing::TestInfo *test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();

    tempDir = std::filesystem::temp_directory_path() / test_info->name();
    tempDirLp = tempDir / "lp";
    if (!std::filesystem::is_directory(tempDirLp)) {
      std::filesystem::create_directories(tempDirLp);
    }

    logger = std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(
        LogUtils::LOGLEVEL::NONE);

    problems_and_data = {
        {std::make_shared<MockProblem>(1),
         ProblemData("problem-1-1--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(1),
         ProblemData("problem-1-50--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(2),
         ProblemData("problem-2-10--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(2),
         ProblemData("problem-2-11--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(2),
         ProblemData("problem-2-30--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(3),
         ProblemData("problem-3-20--optim-nb-1.mps", "variables.txt")},
    };
  }

  void RunWeightsFileWriterTest(const std::string &solver_name,
                                const std::string &expected) {
    auto yearly_weight_writer = YearlyWeightsWriter(
        tempDir, {3, 5, 7}, "weights_123.txt", {1, 2, 3}, solver_name, logger);

    yearly_weight_writer.CreateWeightFile(problems_and_data);

    std::ifstream reader(tempDirLp / "weights_123.txt");
    std::string actual((std::istreambuf_iterator<char>(reader)),
                       std::istreambuf_iterator<char>());

    EXPECT_EQ(expected, actual);
  }

  std::filesystem::path tempDir;
  std::filesystem::path tempDirLp;
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger;
  std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
      problems_and_data;
};

TEST_F(WeightsFileWriterTest, CorrectlyWriteWeightsFileWithXpress) {
  std::string expected = R"xxx(problem-1-1--optim-nb-1.svf 3
problem-1-50--optim-nb-1.svf 3
problem-2-10--optim-nb-1.svf 5
problem-2-11--optim-nb-1.svf 5
problem-2-30--optim-nb-1.svf 5
problem-3-20--optim-nb-1.svf 7
WEIGHT_SUM 15
)xxx";
  RunWeightsFileWriterTest("xpress", expected);
}

TEST_F(WeightsFileWriterTest, CorrectlyWriteWeightsFileWithCoin) {
  std::string expected = R"xxx(problem-1-1--optim-nb-1.mps 3
problem-1-50--optim-nb-1.mps 3
problem-2-10--optim-nb-1.mps 5
problem-2-11--optim-nb-1.mps 5
problem-2-30--optim-nb-1.mps 5
problem-3-20--optim-nb-1.mps 7
WEIGHT_SUM 15
)xxx";
  RunWeightsFileWriterTest("coin", expected);
}
