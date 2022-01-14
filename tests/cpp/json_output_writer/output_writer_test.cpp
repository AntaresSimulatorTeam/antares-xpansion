
#include <ostream>

#include "gtest/gtest.h"

#include "JsonWriter.h"
#include <iostream>
#include <cstdio>
#include <time.h>

using namespace Output;
time_t time_t_from_date(int year, int month, int day, int hour, int min, int sec) ;

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

};

class TimeUtilMock : public Clock{
private:
    int getBeginCount = 0;
    std::time_t _time;
public:
    std::time_t getTime() override{
        getBeginCount ++;
        return _time;
    }
    void set_time(std::time_t time){
        _time = time;
    }
    int get_updateBeginCount() const{
        return getBeginCount;
    }
};


TEST_F(JsonWriterShould, GenerateAValideFile) {
    auto timer= std::make_shared<Clock>();
    auto writer = JsonWriter(timer, std::string());
    auto benders_options = BendersOptions();
    benders_options.JSON_FILE = _fileName;
    writer.initialize(benders_options);
    writer.write_options(benders_options);

    std::ifstream fileStream(_fileName);
    fileStream.close();
    ASSERT_TRUE(fileStream.good() );
}

TEST_F(JsonWriterShould, CallTimerUpdateBeginTime){
    auto timer= std::make_shared<TimeUtilMock>();

    std::time_t my_time_t = time_t_from_date(2020, 1, 1, 12, 10, 30);
    timer->set_time(time_t_from_date(2020, 1, 1, 12, 10, 30));

    auto writer = JsonWriter(timer, _fileName);
    auto benders_options = BendersOptions();

    benders_options.JSON_FILE = _fileName;
    writer.initialize(benders_options);
    int n_calls = timer->get_updateBeginCount();
    ASSERT_EQ(1, n_calls);
    std::string string_time_zero = clock_utils::timeToStr(my_time_t);
    ASSERT_EQ("01-01-2020 12:10:30", string_time_zero);
}

time_t time_t_from_date(int year, int month, int day, int hour, int min, int sec)  {
    time_t current_time;
    struct tm time_info;
    time(&current_time);
    localtime_r(&current_time, &time_info);
    time_info.tm_hour = hour;
    time_info.tm_min = min;
    time_info.tm_sec = sec;
    time_info.tm_mday = day;
    time_info.tm_mon = month-1; //january = 0
    time_info.tm_year = year-1900;
    time_t my_time_t =mktime(&time_info) ;
    return my_time_t;
}

