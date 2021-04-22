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
    }

    void log_at_initialization(const LogData &d) override {
        _initCall = true;
    }

    void log_at_iteration_start(const LogData &d) override {
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
};


TEST(Benders_tests, empty){
    Logger logger = std::make_shared<SimpleLoggerMock>();
    BendersOptions options;
    CouplingMap input;
    // TODO fix this
    Benders benders(input, options,logger);
    ASSERT_TRUE(1);
}
