//
// Created by jmkerloch on 19/04/2021.
//

#include <ostream>

#include "gtest/gtest.h"

#include "benders_sequential_core/ILogger.h"
#include "logger/User.h"
#include "logger/Master.h"

using namespace xpansion::logger;

class UserLoggerTest : public ::testing::Test
{

public :

    UserLoggerTest():
    _logger(_stream)
    {
    }

    void addCandidate(LogData& logData, std::string candidate, double invest, double minInvest, double maxInvest)
    {
        logData.x0[candidate] = invest;
        logData.min_invest[candidate] = minInvest;
        logData.max_invest[candidate] = maxInvest;
    }

    std::stringstream _stream;
    User _logger;
};

TEST_F(UserLoggerTest, EmptyStreamAtInit) {
    ASSERT_EQ( _stream.str().size() ,0 );
}
TEST_F(UserLoggerTest, InitLog) {
    LogData logData;
    _logger.log_at_initialization(logData);
    ASSERT_EQ( _stream.str() ,"" );
}


TEST_F(UserLoggerTest, IterationStartLog) {
    LogData logData;
    addCandidate(logData,"candidate1", 50.0 , 0.0, 100.0);
    addCandidate(logData,"a", 10.0 , 0.0, 50.0);
    addCandidate(logData,"c", 50.0 , 0.0, 100.0);
    addCandidate(logData,"b", 20.0 , 0.0, 200.0);
    addCandidate(logData,"very long name of investment", 50.0 , 0.0, 100.0);

    std::stringstream expected;
    expected << "\t\tCandidates:" << std::endl;
    expected << "\t\t\t                           a = 10.00 invested MW -- possible interval [0.00; 50.00] MW" << std::endl;
    expected << "\t\t\t                           b = 20.00 invested MW -- possible interval [0.00; 200.00] MW" << std::endl;
    expected << "\t\t\t                           c = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;
    expected << "\t\t\t                  candidate1 = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;
    expected << "\t\t\tvery long name of investment = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;

    _logger.log_at_iteration_start(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, EndLog) {
    LogData logData;
    logData.it = 1;
    _logger.log_at_ending(logData);
    ASSERT_EQ( _stream.str() ,"" );
}

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

class MasterLoggerTest : public ::testing::Test
{
public :

    MasterLoggerTest()
    {
        _logger = std::make_shared<SimpleLoggerMock>();
        _logger2 = std::make_shared<SimpleLoggerMock>();
        _master.addLogger(_logger);
        _master.addLogger(_logger2);
    }
    Master _master;
    std::shared_ptr<SimpleLoggerMock> _logger;
    std::shared_ptr<SimpleLoggerMock> _logger2;
};

TEST_F(MasterLoggerTest, InitLog) {
    LogData logData;
    _master.log_at_initialization(logData);
    ASSERT_TRUE( _logger->_initCall);
    ASSERT_TRUE( _logger2->_initCall);
}

TEST_F(MasterLoggerTest, IterationStartLog) {
    LogData logData;
    _master.log_at_iteration_start(logData);
    ASSERT_TRUE( _logger->_iterationStartCall);
    ASSERT_TRUE( _logger2->_iterationStartCall);
}

TEST_F(MasterLoggerTest, IterationEndLog) {
    LogData logData;
    _master.log_at_iteration_end(logData);
    ASSERT_TRUE( _logger->_iterationEndCall);
    ASSERT_TRUE( _logger2->_iterationEndCall);
}

TEST_F(MasterLoggerTest, EndLog) {
    LogData logData;
    _master.log_at_ending(logData);
    ASSERT_TRUE( _logger->_endingCall);
    ASSERT_TRUE( _logger2->_endingCall);
}
