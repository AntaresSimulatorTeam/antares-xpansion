
#include "ILogger.h"
#include "antares-xpansion/benders/benders_core/LastIterationReader.h"
#include "antares-xpansion/benders/benders_core/LastIterationWriter.h"
#include "LoggerStub.h"
#include "antares-xpansion/benders/benders_core/StartUp.h"
#include "WriterStub.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "gtest/gtest.h"
bool operator==(const LogPoint& lhs, const LogPoint& rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

bool operator==(const LogData& lhs, const LogData& rhs) {
  return rhs.lb == lhs.lb && rhs.best_ub == lhs.best_ub && rhs.ub == lhs.ub &&
         rhs.it == lhs.it && rhs.best_it == lhs.best_it &&
         rhs.subproblem_cost == lhs.subproblem_cost &&
         rhs.invest_cost == lhs.invest_cost && rhs.x_in == lhs.x_in &&
         rhs.x_out == lhs.x_out && rhs.x_cut == lhs.x_cut &&
         rhs.min_invest == lhs.min_invest && rhs.max_invest == lhs.max_invest &&
         rhs.optimality_gap == lhs.optimality_gap &&
         rhs.relative_gap == lhs.relative_gap &&
         rhs.max_iterations == lhs.max_iterations &&
         rhs.benders_elapsed_time == lhs.benders_elapsed_time &&
         rhs.master_time == lhs.master_time &&
         rhs.subproblem_time == lhs.subproblem_time &&
         rhs.cumulative_number_of_subproblem_resolved ==
             lhs.cumulative_number_of_subproblem_resolved;
}
const LogPoint best_x_in = {{"candidate1", 56.e8}, {"candidate2", 6e4}};
const LogPoint best_x_out = {{"candidate1", 568.e6}, {"candidate2", 682e9}};
const LogPoint best_x_cut = {{"candidate1", 60e5}, {"candidate2", 12.4e8}};
const LogPoint last_x_in = {{"candidate1", 51.e8}, {"candidate2", 4e4}};
const LogPoint last_x_out = {{"candidate1", 58.e6}, {"candidate2", 62e9}};
const LogPoint last_x_cut = {{"candidate1", 6e5}, {"candidate2", 1.4e8}};
const LogPoint min_invest = {{"candidate1", 68.e6}, {"candidate2", 62e9}};
const LogPoint max_invest = {{"candidate1", 1568.e6}, {"candidate2", 3682e9}};
const LogData best_iteration_data = {
    15e5,   255e6,     200e6,      63,         63,         587e8,
    8562e8, best_x_in, best_x_out, best_x_cut, min_invest, max_invest,
    9e9,    3e-15,     5876,       999,        898,        25};
const LogData last_iteration_data = {
    1e5,    255e6,     200e6,      159,        63,         587e8,
    8562e8, last_x_in, last_x_out, last_x_cut, min_invest, max_invest,
    9e9,    3e-15,     5876,       9999,       898,        23};

class LastIterationReaderTest : public ::testing::Test {
 public:
  LastIterationReaderTest() = default;

  const std::string invalid_file_path = "";
  const std::filesystem::path _last_iteration_file = std::tmpnam(nullptr);
};

TEST_F(LastIterationReaderTest, ShouldFailIfInvalidFileIsGiven) {
  const auto delimiter = std::string("\n");
  std::stringstream expectedErrorString;
  expectedErrorString << delimiter << "Invalid Json file: " << invalid_file_path
                      << delimiter;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto last_ite_reader = LastIterationReader(invalid_file_path);
  std::cerr.rdbuf(initialBufferCerr);
  auto err_str = redirectedErrorStream.str();
  auto first_line_err =
      err_str.substr(0, err_str.find(delimiter, delimiter.length()) + 1);
  ASSERT_EQ(first_line_err, expectedErrorString.str());
}
TEST_F(LastIterationReaderTest,
       ReaderShouldReturnLastAndBestIterationsWrittenByWriter) {
  auto writer = LastIterationWriter(_last_iteration_file);
  writer.SaveBestAndLastIterations(best_iteration_data, last_iteration_data);
  auto reader = LastIterationReader(_last_iteration_file);
  const auto& [last, best] = reader.LastIterationData();

  LogData expec_last_iteration_data = last_iteration_data;
  expec_last_iteration_data.x_in = {};
  expec_last_iteration_data.x_out = {};

  LogData expec_best_iteration_data = best_iteration_data;
  expec_best_iteration_data.x_in = {};
  expec_best_iteration_data.x_out = {};

  ASSERT_TRUE(last == expec_last_iteration_data);
  ASSERT_TRUE(best == expec_best_iteration_data);
}

class LastIterationWriterTest : public ::testing::Test {
 public:
  LastIterationWriterTest() = default;

  const std::string _invalid_file_path = "";
};
TEST_F(LastIterationWriterTest, ShouldFailIfInvalidFileIsGiven) {
  std::stringstream expectedErrorString;
  expectedErrorString << "Invalid Json file: " << _invalid_file_path
                      << std::endl;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto writer = LastIterationWriter(_invalid_file_path);
  writer.SaveBestAndLastIterations(best_iteration_data, last_iteration_data);
  std::cerr.rdbuf(initialBufferCerr);
  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString.str());
}

class WriterMockStatus : public WriterNOOPStub {
 public:
  std::string status = Output::OPTIMAL_C;
  std::string solution_status() const override { return status; }
};

TEST(Resume, ShouldNotResumeIfCriterionOk) {
  Benders::StartUp start_up;
  SimulationOptions options;
  options.MAX_ITERATIONS = 10;
  options.ABSOLUTE_GAP = 100.f;
  options.RELATIVE_GAP = 200.f;
  options.TIME_LIMIT = 500.f;
  options.RESUME = true;

  auto writer = std::make_shared<WriterMockStatus>();
  auto logger = std::make_shared<LoggerNOOPStub>();
  ASSERT_EQ(start_up.StudyAlreadyAchievedCriterion(options, writer, logger),
            true);
}

TEST(Resume, DontResumeIfOptionOff) {
  Benders::StartUp start_up;
  SimulationOptions options;
  options.MAX_ITERATIONS = 10;
  options.ABSOLUTE_GAP = 100.f;
  options.RELATIVE_GAP = 200.f;
  options.TIME_LIMIT = 500.f;
  options.RESUME = false;

  auto writer = std::make_shared<WriterMockStatus>();
  auto logger = std::make_shared<LoggerNOOPStub>();
  ASSERT_EQ(start_up.StudyAlreadyAchievedCriterion(options, writer, logger),
            false);
}

TEST(Resume, ContinueIfCreterionNotMatch) {
  Benders::StartUp start_up;
  SimulationOptions options;
  options.MAX_ITERATIONS = 10;
  options.ABSOLUTE_GAP = 100.f;
  options.RELATIVE_GAP = 200.f;
  options.TIME_LIMIT = 500.f;
  options.RESUME = true;

  auto writer = std::make_shared<WriterMockStatus>();
  writer->status = "NotOptimal";
  auto logger = std::make_shared<LoggerNOOPStub>();
  ASSERT_EQ(start_up.StudyAlreadyAchievedCriterion(options, writer, logger),
            false);
}
