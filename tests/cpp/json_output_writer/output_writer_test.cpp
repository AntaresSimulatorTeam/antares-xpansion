
#include <json/json.h>
#include <time.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>

#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "gtest/gtest.h"

using namespace Output;
time_t time_from_date(int year, int month, int day, int hour, int min, int sec);

const std::string CRIT_ABSOLUTE_GAP_C("absolute gap"),
    CRIT_RELATIVE_GAP_C("relative gap"), CRIT_MAX_ITER_C("maximum iterations"),
    CRIT_TIMELIMIT_C("timelimit"), STOP_ERROR_C("error");
class ClockMock : public Clock {
 private:
  std::time_t _time;

 public:
  std::time_t getTime() override { return _time; }
  void set_time(std::time_t time) { _time = time; }
};

class JsonWriterTest : public ::testing::Test {
 public:
  void SetUp() override { _fileName = std::tmpnam(nullptr); }

  void TearDown() override { std::remove(_fileName.string().c_str()); }

  std::filesystem::path _fileName;
  std::shared_ptr<ClockMock> my_clock = std::make_shared<ClockMock>();
};

TEST_F(JsonWriterTest, GenerateAValideFile) {
  auto writer = JsonWriter(my_clock, _fileName);
  writer.initialize();

  std::ifstream fileStream(_fileName);
  fileStream.close();
  ASSERT_TRUE(fileStream.good());
}

Json::Value get_value_from_json(const std::filesystem::path &file_name) {
  Json::Value _input;
  std::ifstream input_file_l(file_name, std::ifstream::binary);
  Json::CharReaderBuilder builder_l;
  std::string errs;
  if (!parseFromStream(builder_l, input_file_l, &_input, &errs)) {
    throw std::runtime_error(LOGLOCATION);
  }
  return _input;
}

TEST_F(JsonWriterTest, InitialiseShouldPrintBeginTimeAndOptions) {
  // given
  my_clock->set_time(time_from_date(2020, 1, 1, 12, 10, 30));
  auto writer = JsonWriter(my_clock, _fileName);
  // when
  writer.initialize();
  // then
  Json::Value json_content = get_value_from_json(_fileName);
  ASSERT_EQ("01-01-2020 12:10:30", json_content[BEGIN_C].asString());
}

TEST_F(JsonWriterTest, EndWritingShouldPrintEndTimeAndSimuationResults) {
  my_clock->set_time(time_from_date(2020, 1, 1, 12, 10, 30));
  auto writer = JsonWriter(my_clock, _fileName);

  IterationsData iterations_data;
  writer.end_writing(iterations_data);

  Json::Value json_content = get_value_from_json(_fileName);
  ASSERT_EQ("01-01-2020 12:10:30", json_content[END_C].asString());
}

const CandidateData c1 = {"c1", 55, 0.55, 555};
const CandidateData c2 = {"c2", 66, 0.66, 666};
const CandidatesVec cdVec = {c1, c2};
const Iteration iter1 = {15,    1.5, 1.2, 11, 12,    1e-10,
                         1e-12, 1,   17,  20, cdVec, 98};
const CandidateData c3 = {"c3", 33, 0.33, 5553};
const CandidateData c4 = {"c4", 656, 0.4566, 545666};
const CandidatesVec cdVec2 = {c3, c4};
const Iteration iter2 = {105,   2.3, 12, 112, 212,    1e-1,
                         1e-10, 12,  67, 620, cdVec2, 105};

void verify_iteration_in_json_content(const Iteration &iter,
                                      const Json::Value &json_content) {
  ASSERT_EQ(iter.best_ub,
            json_content[ITERATIONS_C]["1"][BEST_UB_C].asDouble());
  ASSERT_EQ(iter.master_duration,
            json_content[ITERATIONS_C]["1"][MASTER_DURATION_C].asDouble());
  ASSERT_EQ(iter.investment_cost,
            json_content[ITERATIONS_C]["1"][INVESTMENT_COST_C].asDouble());
  ASSERT_EQ(iter.lb, json_content[ITERATIONS_C]["1"][LB_C].asDouble());
  ASSERT_EQ(iter.operational_cost,
            json_content[ITERATIONS_C]["1"][OPERATIONAL_COST_C].asDouble());
  ASSERT_EQ(iter.optimality_gap,
            json_content[ITERATIONS_C]["1"][OPTIMALITY_GAP_C].asDouble());
  ASSERT_EQ(iter.relative_gap,
            json_content[ITERATIONS_C]["1"][RELATIVE_GAP_C].asDouble());
  ASSERT_EQ(iter.ub, json_content[ITERATIONS_C]["1"][UB_C].asDouble());
  ASSERT_EQ(iter.cumulative_number_of_subproblem_resolved,
            json_content[ITERATIONS_C]["1"]
                        [CUMULATIVE_NUMBER_OF_SUBPROBLEM_RESOLVED_C]
                            .asInt());

  const auto &cand = iter.candidates.at(0);
  ASSERT_EQ(
      cand.name,
      json_content[ITERATIONS_C]["1"][CANDIDATES_C][0][NAME_C].asString());
  ASSERT_EQ(
      cand.invest,
      json_content[ITERATIONS_C]["1"][CANDIDATES_C][0][INVEST_C].asDouble());
  ASSERT_EQ(cand.min,
            json_content[ITERATIONS_C]["1"][CANDIDATES_C][0][MIN_C].asDouble());
  ASSERT_EQ(cand.max,
            json_content[ITERATIONS_C]["1"][CANDIDATES_C][0][MAX_C].asDouble());
}

void verify_solution_data_in_json_content(const SolutionData &solution_data,
                                          const Json::Value &json_content) {
  ASSERT_EQ(solution_data.solution.investment_cost,
            json_content[SOLUTION_C][INVESTMENT_COST_C].asDouble());
  ASSERT_EQ(solution_data.solution.operational_cost,
            json_content[SOLUTION_C][OPERATIONAL_COST_C].asDouble());
  ASSERT_EQ(solution_data.solution.optimality_gap,
            json_content[SOLUTION_C][OPTIMALITY_GAP_C].asDouble());
  ASSERT_EQ(solution_data.solution.overall_cost,
            json_content[SOLUTION_C][OVERALL_COST_C].asDouble());
  ASSERT_EQ(solution_data.solution.relative_gap,
            json_content[SOLUTION_C][RELATIVE_GAP_C].asDouble());
  ASSERT_EQ(solution_data.best_it,
            json_content[SOLUTION_C][ITERATION_C].asInt());
  ASSERT_EQ(solution_data.problem_status,
            json_content[SOLUTION_C][PROBLEM_STATUS_C].asString());
  ASSERT_EQ(solution_data.stopping_criterion,
            json_content[SOLUTION_C][STOPPING_CRITERION_C].asString());
}

TEST_F(JsonWriterTest, EndWritingShouldPrintIterationsData) {
  auto writer = JsonWriter(my_clock, _fileName);

  Iterations itersVec = {iter1, iter2};

  SolutionData solution_data;
  solution_data.solution = iter2;
  solution_data.nbWeeks_p = 5;
  solution_data.best_it = 2;
  solution_data.problem_status = OPTIMAL_C;
  solution_data.stopping_criterion = CRIT_MAX_ITER_C;

  IterationsData iterations_data;
  iterations_data.nbWeeks_p = 5;
  iterations_data.elapsed_time = 55;
  iterations_data.iters = itersVec;
  iterations_data.solution_data = solution_data;

  const int expected_log_level = 1;
  const std::string expected_master_name = "Name";
  const std::string expected_solver_name = "Solver";
  writer.write_log_level(expected_log_level);
  writer.write_master_name(expected_master_name);
  writer.write_solver_name(expected_solver_name);
  writer.end_writing(iterations_data);

  Json::Value json_content = get_value_from_json(_fileName);
  verify_iteration_in_json_content(iter1, json_content);
  verify_solution_data_in_json_content(solution_data, json_content);

  ASSERT_EQ(expected_log_level, json_content[OPTIONS_C][LOG_LEVEL_C].asInt());
  ASSERT_EQ(expected_master_name,
            json_content[OPTIONS_C][MASTER_NAME_C].asString());
  ASSERT_EQ(expected_solver_name,
            json_content[OPTIONS_C][SOLVER_NAME_C].asString());
}
time_t time_from_date(int year, int month, int day, int hour, int min,
                      int sec) {
  time_t current_time;
  struct tm time_info;
  time(&current_time);
  localtime_platform(current_time, time_info);
  time_info.tm_hour = hour;
  time_info.tm_min = min;
  time_info.tm_sec = sec;
  time_info.tm_mday = day;
  time_info.tm_mon = month - 1;  // january = 0
  time_info.tm_year = year - 1900;
  time_t my_time_t = mktime(&time_info);
  return my_time_t;
}
TEST_F(JsonWriterTest, WriteIterationShouldPrintIterationInOutfile) {
  auto writer = JsonWriter(my_clock, _fileName);
  writer.initialize();
  writer.write_iteration(iter1, 1);
  writer.dump();
  Json::Value json_content = get_value_from_json(_fileName);
  verify_iteration_in_json_content(iter1, json_content);
}
TEST_F(JsonWriterTest, WriterShouldPreserveAlreadyWrittenIterationsInOutfile) {
  auto writer = JsonWriter(my_clock, _fileName);
  writer.initialize();
  writer.write_iteration(iter1, 1);
  writer.dump();
  Json::Value json_content = get_value_from_json(_fileName);
  verify_iteration_in_json_content(iter1, json_content);
  auto writer2 = JsonWriter(_fileName, json_content);
  writer2.write_iteration(iter2, 2);
  Json::Value json_content2 = get_value_from_json(_fileName);

  verify_iteration_in_json_content(iter1, json_content2);
}