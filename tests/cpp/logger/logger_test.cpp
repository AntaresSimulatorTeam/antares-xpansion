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

TEST_F(UserLoggerTest, IterationLog) {
    LogData logData;
    logData.it = 1;
    std::string expected = "it = 1\n";
    _logger.log_at_iteration(logData);
    ASSERT_EQ( _stream.str() ,expected );
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
        _iterationCall = false;
        _endingCall = false;
    }

    void log_at_initialization(const LogData &d) override {
        _initCall = true;
    }

    void log_at_iteration(const LogData &d) override {
        _iterationCall = true;
    }

    void log_at_ending(const LogData &d) override {
        _endingCall = true;
    }

    bool _initCall;
    bool _iterationCall;
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

TEST_F(MasterLoggerTest, IterationLog) {
    LogData logData;
    _master.log_at_iteration(logData);
    ASSERT_TRUE( _logger->_iterationCall);
    ASSERT_TRUE( _logger2->_iterationCall);
}

TEST_F(MasterLoggerTest, EndLog) {
    LogData logData;
    _master.log_at_ending(logData);
    ASSERT_TRUE( _logger->_endingCall);
    ASSERT_TRUE( _logger2->_endingCall);
}
