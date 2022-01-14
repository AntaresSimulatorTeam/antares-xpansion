
#include <ostream>

#include "gtest/gtest.h"

#include "JsonWriter.h"
#include <iostream>
#include <cstdio>
#include <time.h>
#include "gmock/gmock.h"

using namespace Output;

class JsonWriterShould : public ::testing::Test
{
public:

    void SetUp()
    {
        _fileName = std::tmpnam(nullptr);
    }

    void TearDown()
    {
        std::remove(_fileName.c_str());
    }


    std::string _fileName;

};

class TimeUtilMock : public TimeUtil{
private:
    int updateBeginCount = 0;
public:
    void updateBeginTime(){
        updateBeginCount ++;
    }
    int get_updateBeginCount() const{
        return updateBeginCount;
    }
};


TEST_F(JsonWriterShould, GenerateAValideFile) {
    auto timer= std::make_shared<TimeUtil>();
    auto writer = JsonWriter(timer);
    auto benders_options = BendersOptions();
    benders_options.JSON_FILE = _fileName;
    writer.initialize(benders_options);
    writer.write_options(benders_options);

    std::ifstream fileStream(_fileName);
    fileStream.close();
    ASSERT_TRUE(fileStream.good() );
}

TEST_F(JsonWriterShould, CallTimerUpdateBeginTime) {
    auto timer= std::make_shared<TimeUtilMock>();
    auto writer = JsonWriter(timer);
    auto benders_options = BendersOptions();
    benders_options.JSON_FILE = _fileName;
    writer.initialize(benders_options);
    writer.write_options(benders_options);
    int n_calls = timer->get_updateBeginCount();
    ASSERT_EQ(1, n_calls);
}

