
#include <ostream>

#include "gtest/gtest.h"

#include "JsonWriter.h"
#include <iostream>
#include <cstdio>
#include <time.h>
#include <json/json.h>

using namespace Output;
time_t time_from_date(int year, int month, int day, int hour, int min, int sec);

class ClockMock : public Clock
{
private:
    std::time_t _time;

public:
    std::time_t getTime() override
    {
        return _time;
    }
    void set_time(std::time_t time)
    {
        _time = time;
    }
};

class JsonWriterTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        _fileName = std::tmpnam(nullptr);
    }

    void TearDown() override
    {
        std::remove(_fileName.c_str());
    }

    std::string _fileName;
    std::shared_ptr<ClockMock> my_clock = std::make_shared<ClockMock>();
};

TEST_F(JsonWriterTest, GenerateAValideFile)
{
    auto writer = JsonWriter(my_clock, _fileName);
    auto benders_options = BendersOptions();
    writer.initialize(benders_options);
    writer.write_options(benders_options);

    std::ifstream fileStream(_fileName);
    fileStream.close();
    ASSERT_TRUE(fileStream.good());
}

Json::Value get_value_from_json(const std::string &file_name)
{
    Json::Value _input;
    std::ifstream input_file_l(file_name, std::ifstream::binary);
    Json::CharReaderBuilder builder_l;
    std::string errs;
    if (!parseFromStream(builder_l, input_file_l, &_input, &errs))
    {
        throw std::runtime_error("");
    }
    return _input;
}

TEST_F(JsonWriterTest, InitialiseShouldPrintBeginTimeAndOptions)
{
    // given
    my_clock->set_time(time_from_date(2020, 1, 1, 12, 10, 30));
    auto writer = JsonWriter(my_clock, _fileName);
    // when
    auto benders_options = BendersOptions();
    writer.initialize(benders_options);
    // then
    Json::Value json_content = get_value_from_json(_fileName);
    ASSERT_EQ("01-01-2020 12:10:30", json_content["begin"].asString());
    ASSERT_EQ(benders_options.LOG_LEVEL, json_content["options"]["LOG_LEVEL"].asInt());
    ASSERT_EQ(benders_options.MAX_ITERATIONS, json_content["options"]["MAX_ITERATIONS"].asInt());
    ASSERT_EQ(benders_options.ABSOLUTE_GAP, json_content["options"]["ABSOLUTE_GAP"].asDouble());
    ASSERT_EQ(benders_options.RELATIVE_GAP, json_content["options"]["RELATIVE_GAP"].asDouble());
    ASSERT_EQ(benders_options.AGGREGATION, json_content["options"]["AGGREGATION"].asBool());
    ASSERT_EQ(benders_options.OUTPUTROOT, json_content["options"]["OUTPUTROOT"].asString());
    ASSERT_EQ(benders_options.TRACE, json_content["options"]["TRACE"].asBool());
    ASSERT_EQ(benders_options.DELETE_CUT, json_content["options"]["DELETE_CUT"].asBool());
    ASSERT_EQ(benders_options.SLAVE_WEIGHT, json_content["options"]["SLAVE_WEIGHT"].asString());
    ASSERT_EQ(benders_options.MASTER_NAME, json_content["options"]["MASTER_NAME"].asString());
    ASSERT_EQ(benders_options.SLAVE_NUMBER, json_content["options"]["SLAVE_NUMBER"].asInt());
    ASSERT_EQ(benders_options.INPUTROOT, json_content["options"]["INPUTROOT"].asString());
    ASSERT_EQ(benders_options.BASIS, json_content["options"]["BASIS"].asBool());
    ASSERT_EQ(benders_options.ACTIVECUTS, json_content["options"]["ACTIVECUTS"].asBool());
    ASSERT_EQ(benders_options.THRESHOLD_AGGREGATION, json_content["options"]["THRESHOLD_AGGREGATION"].asInt());
    ASSERT_EQ(benders_options.THRESHOLD_ITERATION, json_content["options"]["THRESHOLD_ITERATION"].asInt());
    ASSERT_EQ(benders_options.CSV_NAME, json_content["options"]["CSV_NAME"].asString());
    ASSERT_EQ(benders_options.BOUND_ALPHA, json_content["options"]["BOUND_ALPHA"].asBool());
    ASSERT_EQ(benders_options.SOLVER_NAME, json_content["options"]["SOLVER_NAME"].asString());
    ASSERT_EQ(benders_options.JSON_FILE, json_content["options"]["JSON_FILE"].asString());
    ASSERT_EQ(benders_options.TIME_LIMIT, json_content["options"]["TIME_LIMIT"].asDouble());
}

TEST_F(JsonWriterTest, EndWritingShouldPrintEndTimeAndSimuationResults)
{
    my_clock->set_time(time_from_date(2020, 1, 1, 12, 10, 30));
    auto writer = JsonWriter(my_clock, _fileName);

    IterationsData iterations_data;
    writer.end_writing(iterations_data);

    Json::Value json_content = get_value_from_json(_fileName);
    ASSERT_EQ("01-01-2020 12:10:30", json_content["end"].asString());
}

TEST_F(JsonWriterTest, EndWritingShouldPrintIterationsData)
{
    auto writer = JsonWriter(my_clock, _fileName);

    CandidateData c1, c2;
    c1.name = "c1";
    c1.invest = 55;
    c1.min = 0.55;
    c1.max = 555;
    c2.name = "c2";
    c2.invest = 66;
    c2.min = 0.66;
    c2.max = 666;
    CandidatesVec cdVec = {c1, c2};

    Iteration iter1;
    iter1.time = 15;
    iter1.lb = 1.2;
    iter1.best_ub = 12;
    iter1.optimality_gap = 1e-10;
    iter1.relative_gap = 1e-12;
    iter1.investment_cost = 1;
    iter1.operational_cost = 17;
    iter1.overall_cost = 20;
    iter1.candidates = cdVec;

    CandidateData c3, c4;
    c3.name = "c3";
    c3.invest = 33;
    c3.min = 0.33;
    c3.max = 5553;
    c4.name = "c4";
    c4.invest = 656;
    c4.min = 0.4566;
    c4.max = 545666;
    CandidatesVec cdVec2 = {c3, c4};

    Iteration iter2;
    iter2.time = 105;
    iter2.lb = 12;
    iter2.best_ub = 212;
    iter2.optimality_gap = 1e-1;
    iter2.relative_gap = 1e-10;
    iter2.investment_cost = 12;
    iter2.operational_cost = 67;
    iter2.overall_cost = 620;
    iter2.candidates = cdVec2;

    Iterations itersVec = {iter1, iter2};

    SolutionData solution_data;
    solution_data.solution = iter2;
    solution_data.nbWeeks_p = 5;
    solution_data.best_it = 2;
    solution_data.problem_status = "OPTIMAL";
    solution_data.stopping_criterion = StoppingCriterion::max_iteration;

    IterationsData iterations_data;
    iterations_data.nbWeeks_p = 5;
    iterations_data.elapsed_time = 55;
    iterations_data.iters = itersVec;
    iterations_data.solution_data = solution_data;

    writer.end_writing(iterations_data);

    Json::Value json_content = get_value_from_json(_fileName);

    ASSERT_EQ(iter1.best_ub, json_content["iterations"]["1"]["best_ub"].asDouble());
    ASSERT_EQ(iter1.time, json_content["iterations"]["1"]["duration"].asDouble());
    ASSERT_EQ(iter1.investment_cost, json_content["iterations"]["1"]["investment_cost"].asDouble());
    ASSERT_EQ(iter1.lb, json_content["iterations"]["1"]["lb"].asDouble());
    ASSERT_EQ(iter1.operational_cost, json_content["iterations"]["1"]["operational_cost"].asDouble());
    ASSERT_EQ(iter1.optimality_gap, json_content["iterations"]["1"]["optimality_gap"].asDouble());
    ASSERT_EQ(iter1.relative_gap, json_content["iterations"]["1"]["relative_gap"].asDouble());
    ASSERT_EQ(iter1.ub, json_content["iterations"]["1"]["ub"].asDouble());

    // ASSERT_EQ(iter1.ub, json_content["iterations"]["1"]["candidates"][""].asDouble());
}
time_t time_from_date(int year, int month, int day, int hour, int min, int sec)
{
    time_t current_time;
    struct tm time_info;
    time(&current_time);
    localtime_platform(current_time, time_info);
    time_info.tm_hour = hour;
    time_info.tm_min = min;
    time_info.tm_sec = sec;
    time_info.tm_mday = day;
    time_info.tm_mon = month - 1; // january = 0
    time_info.tm_year = year - 1900;
    time_t my_time_t = mktime(&time_info);
    return my_time_t;
}
