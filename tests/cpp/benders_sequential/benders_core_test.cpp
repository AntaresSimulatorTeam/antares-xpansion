//
// Created by sgatto on 22/04/2021.
//


#include <ostream>

#include "gtest/gtest.h"

#include "benders_sequential_core/Benders.h"


class SimpleLoggerMock : public ILogger
{
public:

    SimpleLoggerMock()
    {
        _initCall = false;
        _iterationStartCall = false;
        _iterationEndCall = false;
        _endingCall = false;

        _durationInSecond = std::nan("");
    }

    void display_message(const std::string& str) {
        _displaymessage = str;
    }

    void display_process_duration(const std::string& processName, double durationInSeconds) {
        _processName = processName;
        _durationInSecond = durationInSeconds;
    }

    void log_at_initialization(const LogData &d) override {
        _initCall = true;
    }

    void log_iteration_candidates(const LogData &d) override {
        _iterationStartCall = true;
    }

    void log_at_iteration_end(const LogData &d) override {
        _iterationEndCall = true;
    }

    void log_at_ending(const LogData &d) override {
        _endingCall = true;
    }

    bool _initCall;
    bool _iterationStartCall;
    bool _iterationEndCall;
    bool _endingCall;
    std::string _displaymessage;
    std::string _processName; double _durationInSecond;
};


TEST(Benders_tests, empty){
    Logger logger = std::make_shared<SimpleLoggerMock>();
    BendersOptions options;
    CouplingMap input;
    // TODO fix this
    Benders benders(input, options,logger);
    ASSERT_TRUE(1);
}
