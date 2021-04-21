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

    const std::string indent_0 = "\t\t";
    const std::string indent_1 = "\t";

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


TEST_F(UserLoggerTest, IterationStartLogCandidateOrder) {
    LogData logData;
    addCandidate(logData,"z", 50.0 , 0.0, 100.0);
    addCandidate(logData,"a", 10.0 , 0.0, 50.0);
    addCandidate(logData,"c", 50.0 , 0.0, 100.0);

    std::stringstream expected;
    expected << indent_0 << "Candidates:" << std::endl;
    expected << indent_0 << indent_1 << "a = 10.00 invested MW -- possible interval [0.00;  50.00] MW" << std::endl;
    expected << indent_0 << indent_1 << "c = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;
    expected << indent_0 << indent_1 << "z = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;

    _logger.log_at_iteration_start(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, IterationStartLogCandidateLongInvestment) {
    LogData logData;
    addCandidate(logData,"z", 5000000.0 , 0.0, 10000000.0);
    addCandidate(logData,"a", 10.0 , 200.0, 50.0);
    addCandidate(logData,"b", 20.0 , 0.0, 200.0);

    std::stringstream expected;
    expected << "\t\tCandidates:" << std::endl;
    expected << indent_0 << indent_1 << "a =      10.00 invested MW -- possible interval [200.00;       50.00] MW" << std::endl;
    expected << indent_0 << indent_1 << "b =      20.00 invested MW -- possible interval [  0.00;      200.00] MW" << std::endl;
    expected << indent_0 << indent_1 << "z = 5000000.00 invested MW -- possible interval [  0.00; 10000000.00] MW" << std::endl;

    _logger.log_at_iteration_start(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, IterationStartLogCandidateNameLenght) {
    LogData logData;
    addCandidate(logData,"z", 50.0 , 0.0, 100.0);
    addCandidate(logData,"a", 10.0 , 0.0, 50.0);
    addCandidate(logData,"very long name of investment", 50.0 , 0.0, 100.0);

    std::stringstream expected;
    expected << "\t\tCandidates:" << std::endl;
    expected << indent_0 << indent_1 << "                           a = 10.00 invested MW -- possible interval [0.00;  50.00] MW" << std::endl;
    expected << indent_0 << indent_1 << "very long name of investment = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;
    expected << indent_0 << indent_1 << "                           z = 50.00 invested MW -- possible interval [0.00; 100.00] MW" << std::endl;

    _logger.log_at_iteration_start(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, IterationEndLog) {
    LogData logData;
    logData.slave_cost = 15.5e6;
    logData.invest_cost = 20e6;
    logData.best_ub = 3e6;
    logData.lb = 2e6;

    std::stringstream expected;
    expected << indent_0 << "Solution =" << std::endl;
    expected << indent_0 << indent_1 << "Operational cost =       15.50 Me" << std::endl;
    expected << indent_0 << indent_1 << " Investment cost =       20.00 Me" << std::endl;
    expected << indent_0 << indent_1 << "    Overall cost =       35.50 Me" << std::endl;
    expected << indent_0 << indent_1 << "   Best Solution =        3.00 Me" << std::endl;
    expected << indent_0 << indent_1 << "     Lower Bound =        2.00 Me" << std::endl;
    expected << indent_0 << indent_1 << "             Gap = 1.00000e+06 e" << std::endl;

    _logger.log_at_iteration_end(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, IterationEndLogLongCost) {
    LogData logData;
    logData.slave_cost = 150000000.5e6;
    logData.invest_cost = 200000000e6;
    logData.best_ub = 100e6;
    logData.lb = 3e6;

    std::stringstream expected;
    expected << indent_0 << "Solution =" << std::endl;
    expected << indent_0 << indent_1 << "Operational cost = 150000000.50 Me" << std::endl;
    expected << indent_0 << indent_1 << " Investment cost = 200000000.00 Me" << std::endl;
    expected << indent_0 << indent_1 << "    Overall cost = 350000000.50 Me" << std::endl;
    expected << indent_0 << indent_1 << "   Best Solution =       100.00 Me" << std::endl;
    expected << indent_0 << indent_1 << "     Lower Bound =         3.00 Me" << std::endl;
    expected << indent_0 << indent_1 << "             Gap =  9.70000e+07 e" << std::endl;

    _logger.log_at_iteration_end(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, EndLogWithinOptimitality) {
    LogData logData;
    logData.best_it = 1;
    logData.best_ub = 20;
    logData.lb = 19.5;
    logData.slave_cost = 1e6;
    logData.invest_cost = 10e6;
    logData.optimal_gap = 1;

    std::stringstream expected;
    expected << "--- CONVERGENCE within optimitality gap :" << std::endl;
    expected << indent_1 << "Best solution = it 1" << std::endl;
    expected << indent_1 << " Overall cost = 11.00 Me" << std::endl;

    _logger.log_at_ending(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
}

TEST_F(UserLoggerTest, EndLogOutsideOptimitality) {
    LogData logData;
    logData.best_it = 1;
    logData.best_ub = 20;
    logData.lb = 18;
    logData.slave_cost = 1e6;
    logData.invest_cost = 10e6;
    logData.optimal_gap = 1;

    std::stringstream expected;
    expected << "--- CONVERGENCE outside optimitality gap :" << std::endl;
    expected << indent_1 << "Best solution = it 1" << std::endl;
    expected << indent_1 << " Overall cost = 11.00 Me" << std::endl;

    _logger.log_at_ending(logData);
    ASSERT_EQ( _stream.str() ,expected.str() );
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
