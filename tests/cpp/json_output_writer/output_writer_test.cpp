
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

class JsonWriterShould : public ::testing::Test
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

TEST_F(JsonWriterShould, GenerateAValideFile)
{
    auto timer = std::make_shared<Clock>();
    auto writer = JsonWriter(timer, _fileName);
    auto benders_options = BendersOptions();
    benders_options.JSON_FILE = _fileName;
    writer.initialize(benders_options);
    writer.write_options(benders_options);

    std::ifstream fileStream(_fileName);
    fileStream.close();
    ASSERT_TRUE(fileStream.good());
}

Json::Value get_value_from_json(const std::string& file_name){
    Json::Value _input;
    std::ifstream input_file_l(file_name, std::ifstream::binary);
    Json::CharReaderBuilder builder_l;
    std::string errs;
    if (!parseFromStream(builder_l, input_file_l, &_input, &errs)) {
        throw std::runtime_error("");
    }
    return _input;
}
TEST_F(JsonWriterShould, PrintBeginTime)
{
    my_clock->set_time(time_from_date(2020, 1, 1, 12, 10, 30));

    auto writer = JsonWriter(my_clock, _fileName);
    auto benders_options = BendersOptions();

    benders_options.JSON_FILE = _fileName;
    writer.initialize(benders_options);
    writer.dump();
    Json::Value json_content = get_value_from_json(_fileName);
    ASSERT_EQ("01-01-2020 12:10:30", json_content["begin"].asString());
    IterationsData iterations_data;
    my_clock->set_time(time_from_date(2020, 2, 2, 12, 10, 30));
    writer.end_writing(iterations_data);
    json_content = get_value_from_json(_fileName);
    ASSERT_EQ("01-01-2020 12:10:30", json_content["begin"].asString());
    ASSERT_EQ("02-02-2020 12:10:30", json_content["end"].asString());
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
