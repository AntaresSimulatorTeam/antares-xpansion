
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
