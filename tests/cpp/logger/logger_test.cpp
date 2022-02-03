#include <cstdio>
#include <iostream>
#include <ostream>

#include "core/ILogger.h"
#include "gtest/gtest.h"
#include "logger/Master.h"
#include "logger/User.h"
#include "logger/UserFile.h"

using namespace xpansion::logger;

class FileLoggerTest : public ::testing::Test {
 public:
  void SetUp() { _fileName = std::tmpnam(nullptr); }

  void TearDown() { std::remove(_fileName.c_str()); }

  std::string _fileName;
};

TEST_F(FileLoggerTest, InvalidFileNotified) {
  const std::string &expectedErrorString =
      "Invalid file name passed as parameter";
  std::stringstream redirectedErrorStream;
  std::streambuf *initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());

  UserFile userFileLogger("");
  std::cerr.rdbuf(initialBufferCerr);

  ASSERT_TRUE(redirectedErrorStream.str().find(expectedErrorString) !=
              std::string::npos);
}

TEST_F(FileLoggerTest, EmptyFileCreatedAtInit) {
  UserFile fileLog(_fileName);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;

  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();

  ASSERT_TRUE(fileStream.good() && stringStreamFromFile.str().empty());
}

TEST_F(FileLoggerTest, FileHasBeenWritten) {
  UserFile fileLog(_fileName);
  const std::string &displayMessage = "Writing test";
  std::stringstream expected;
  expected << displayMessage << std::endl;

  fileLog.display_message(displayMessage);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStringFromFile;

  stringStringFromFile << fileStream.rdbuf();
  fileStream.close();

  ASSERT_EQ(stringStringFromFile.str(), expected.str());
}

class UserLoggerTest : public ::testing::Test {
 public:
  const std::string indent_0 = "\t\t";
  const std::string indent_1 = "\t";

  UserLoggerTest() : _logger(_stream) {}

  void addCandidate(LogData &logData, std::string candidate, double invest,
                    double minInvest, double maxInvest) {
    logData.x0[candidate] = invest;
    logData.min_invest[candidate] = minInvest;
    logData.max_invest[candidate] = maxInvest;
  }

  std::stringstream _stream;
  User _logger;
};

TEST_F(UserLoggerTest, EmptyStreamAtInit) {
  ASSERT_EQ(_stream.str().size(), 0);
}

TEST_F(UserLoggerTest, InvalidStreamNotified) {
  const std::string expectedErrorString =
      "Invalid stream passed as parameter\n";

  std::stringstream redirectedErrorStream;
  std::streambuf *initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  std::ofstream invalidStream("");

  User userLogger(invalidStream);

  std::cerr.rdbuf(initialBufferCerr);

  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString);
}

TEST_F(UserLoggerTest, InitLog) {
  LogData logData;
  logData.it = 1;
  std::stringstream expected;
  expected << "ITERATION 1:" << std::endl;

  _logger.log_at_initialization(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, IterationStartLogCandidateOrder) {
  LogData logData;
  addCandidate(logData, "z", 50.0, 0.0, 100.0);
  addCandidate(logData, "a", 10.0, 0.0, 50.0);
  addCandidate(logData, "c", 50.0, 0.0, 100.0);

  std::stringstream expected;
  expected << indent_0 << "Candidates:" << std::endl;
  expected << indent_0 << indent_1
           << "a = 10.00 invested MW -- possible interval [0.00;  50.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "c = 50.00 invested MW -- possible interval [0.00; 100.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "z = 50.00 invested MW -- possible interval [0.00; 100.00] MW"
           << std::endl;

  _logger.log_iteration_candidates(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, IterationStartLogCandidateLongInvestment) {
  LogData logData;
  addCandidate(logData, "z", 5000000.0, 0.0, 10000000.0);
  addCandidate(logData, "a", 10.0, 200.0, 50.0);
  addCandidate(logData, "b", 20.0, 0.0, 200.0);

  std::stringstream expected;
  expected << indent_0 << "Candidates:" << std::endl;
  expected << indent_0 << indent_1
           << "a =      10.00 invested MW -- possible interval [200.00;       "
              "50.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "b =      20.00 invested MW -- possible interval [  0.00;      "
              "200.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "z = 5000000.00 invested MW -- possible interval [  0.00; "
              "10000000.00] MW"
           << std::endl;

  _logger.log_iteration_candidates(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, IterationStartLogCandidateNameLenght) {
  LogData logData;
  addCandidate(logData, "z", 50.0, 0.0, 100.0);
  addCandidate(logData, "a", 10.0, 0.0, 50.0);
  addCandidate(logData, "very long name of investment", 50.0, 0.0, 100.0);

  std::stringstream expected;
  expected << indent_0 << "Candidates:" << std::endl;
  expected << indent_0 << indent_1
           << "                           a = 10.00 invested MW -- possible "
              "interval [0.00;  50.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "very long name of investment = 50.00 invested MW -- possible "
              "interval [0.00; 100.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "                           z = 50.00 invested MW -- possible "
              "interval [0.00; 100.00] MW"
           << std::endl;

  _logger.log_iteration_candidates(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, IterationEndLog) {
  LogData logData;
  logData.slave_cost = 15.5e6;
  logData.invest_cost = 20e6;
  logData.best_ub = 3e6;
  logData.lb = 2e6;

  std::stringstream expected;
  expected << indent_0 << "Solution =" << std::endl;
  expected << indent_0 << indent_1 << "Operational cost =       15.50 Me"
           << std::endl;
  expected << indent_0 << indent_1 << " Investment cost =       20.00 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "    Overall cost =       35.50 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "   Best Solution =        3.00 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "     Lower Bound =        2.00 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "    Absolute gap = 1.00000e+06 e"
           << std::endl;
  expected << indent_0 << indent_1 << "    Relative gap = 3.33333e-01"
           << std::endl;

  _logger.log_at_iteration_end(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, IterationEndLogLongCost) {
  LogData logData;
  logData.slave_cost = 150000000.5e6;
  logData.invest_cost = 200000000e6;
  logData.best_ub = 100e6;
  logData.lb = 3e6;

  std::stringstream expected;
  expected << indent_0 << "Solution =" << std::endl;
  expected << indent_0 << indent_1 << "Operational cost = 150000000.50 Me"
           << std::endl;
  expected << indent_0 << indent_1 << " Investment cost = 200000000.00 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "    Overall cost = 350000000.50 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "   Best Solution =       100.00 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "     Lower Bound =         3.00 Me"
           << std::endl;
  expected << indent_0 << indent_1 << "    Absolute gap =  9.70000e+07 e"
           << std::endl;
  expected << indent_0 << indent_1 << "    Relative gap =  9.70000e-01"
           << std::endl;

  _logger.log_at_iteration_end(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, MaxIterationsReached) {
  std::stringstream expected;
  expected << "--- Run completed: maximum iterations reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::max_iteration;
  _logger.log_stop_criterion_reached(criterion);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, TimeLimitReached) {
  std::stringstream expected;
  expected << "--- Run completed: timelimit reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::timelimit;
  _logger.log_stop_criterion_reached(criterion);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, AbsoluteGapReached) {
  std::stringstream expected;
  expected << "--- Run completed: absolute gap reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::absolute_gap;
  _logger.log_stop_criterion_reached(criterion);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, RelativeGapReached) {
  std::stringstream expected;
  expected << "--- Run completed: relative gap reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::relative_gap;
  _logger.log_stop_criterion_reached(criterion);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, EndLog) {
  LogData logData;
  logData.best_it = 1;
  logData.best_ub = 20;
  logData.lb = 19.5;
  logData.slave_cost = 1e6;
  logData.invest_cost = 10e6;
  logData.optimality_gap = 1;
  logData.relative_gap = 1e-6;
  logData.it = 2;
  logData.max_iterations = 10;
  std::stringstream expected;
  expected << indent_1 << "Best solution = it 1" << std::endl;
  expected << indent_1 << " Overall cost = 11.00 Me" << std::endl;

  _logger.log_at_ending(logData);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, DifferentCallsAddToTheSamStream) {
  LogData logData1;
  logData1.it = 1;
  addCandidate(logData1, "z", 5000000.0, 0.0, 10000000.0);
  addCandidate(logData1, "a", 10.0, 200.0, 50.0);

  std::stringstream expected;
  expected << indent_0 << "Candidates:" << std::endl;
  expected << indent_0 << indent_1
           << "a =      10.00 invested MW -- possible interval [200.00;       "
              "50.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "z = 5000000.00 invested MW -- possible interval [  0.00; "
              "10000000.00] MW"
           << std::endl;

  LogData logData2;
  logData2.it = 2;
  addCandidate(logData2, "b", 6000000.0, 0.0, 10000000.0);
  addCandidate(logData2, "a", 200.0, 200.0, 50.0);

  expected << indent_0 << "Candidates:" << std::endl;
  expected << indent_0 << indent_1
           << "a =     200.00 invested MW -- possible interval [200.00;       "
              "50.00] MW"
           << std::endl;
  expected << indent_0 << indent_1
           << "b = 6000000.00 invested MW -- possible interval [  0.00; "
              "10000000.00] MW"
           << std::endl;
  _logger.log_iteration_candidates(logData1);
  _logger.log_iteration_candidates(logData2);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, DisplayMessage) {
  std::stringstream expected;
  expected << "Message" << std::endl;

  _logger.display_message("Message");
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, LogMasterDuration) {
  std::stringstream expected;
  expected << indent_1 << "Master solved in 3 s" << std::endl;
  _logger.log_master_solving_duration(3.000000);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, LogSubProblemDuration) {
  std::stringstream expected;
  expected << indent_1 << "Subproblems solved in 3 s" << std::endl;
  _logger.log_subproblems_solving_duration(3.000000);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(UserLoggerTest, LogTotalDuration) {
  std::stringstream expected;
  expected << "Problem ran in 3 s" << std::endl;
  _logger.log_total_duration(3.000000);
  ASSERT_EQ(_stream.str(), expected.str());
}

class SimpleLoggerMock : public ILogger {
 public:
  SimpleLoggerMock() {
    _initCall = false;
    _iterationStartCall = false;
    _iterationEndCall = false;
    _endingCall = false;

    _stopping_criterion = StoppingCriterion::empty;

    _durationMaster = 0.0;
    _durationSubproblem = 0.0;
    _durationTotal = 0.0;
  }

  void display_message(const std::string &str) { _displaymessage = str; }

  void log_at_initialization(const LogData &d) override { _initCall = true; }

  void log_iteration_candidates(const LogData &d) override {
    _iterationStartCall = true;
  }

  void log_master_solving_duration(double durationInSeconds) {
    _durationMaster = durationInSeconds;
  }

  void log_subproblems_solving_duration(double durationInSeconds) {
    _durationSubproblem = durationInSeconds;
  }

  void log_at_iteration_end(const LogData &d) override {
    _iterationEndCall = true;
  }

  void log_at_ending(const LogData &d) override { _endingCall = true; }
  void log_total_duration(double durationInSeconds) {
    _durationTotal = durationInSeconds;
  }

  void log_stop_criterion_reached(
      const StoppingCriterion stopping_criterion) override {
    _stopping_criterion = stopping_criterion;
  }

  bool _initCall;
  bool _iterationStartCall;
  bool _iterationEndCall;
  bool _endingCall;
  StoppingCriterion _stopping_criterion;

  std::string _displaymessage;

  double _durationMaster;
  double _durationSubproblem;
  double _durationTotal;
};

class MasterLoggerTest : public ::testing::Test {
 public:
  MasterLoggerTest() {
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
  ASSERT_TRUE(_logger->_initCall);
  ASSERT_TRUE(_logger2->_initCall);
}

TEST_F(MasterLoggerTest, IterationStartLog) {
  LogData logData;
  _master.log_iteration_candidates(logData);
  ASSERT_TRUE(_logger->_iterationStartCall);
  ASSERT_TRUE(_logger2->_iterationStartCall);
}

TEST_F(MasterLoggerTest, IterationEndLog) {
  LogData logData;
  _master.log_at_iteration_end(logData);
  ASSERT_TRUE(_logger->_iterationEndCall);
  ASSERT_TRUE(_logger2->_iterationEndCall);
}

TEST_F(MasterLoggerTest, EndLog) {
  LogData logData;
  _master.log_at_ending(logData);
  ASSERT_TRUE(_logger->_endingCall);
  ASSERT_TRUE(_logger2->_endingCall);
}

TEST_F(MasterLoggerTest, DisplayMessage) {
  std::string message = "message";
  _master.display_message(message);
  ASSERT_EQ(_logger->_displaymessage, message);
  ASSERT_EQ(_logger2->_displaymessage, message);
}

TEST_F(MasterLoggerTest, LogMasterDuration) {
  double duration = 3.0;
  _master.log_master_solving_duration(duration);
  ASSERT_EQ(_logger->_durationMaster, duration);
  ASSERT_EQ(_logger2->_durationMaster, duration);
}

TEST_F(MasterLoggerTest, LogSubProblemDuration) {
  double duration = 3.0;
  _master.log_subproblems_solving_duration(duration);
  ASSERT_EQ(_logger->_durationSubproblem, duration);
  ASSERT_EQ(_logger2->_durationSubproblem, duration);
}

TEST_F(MasterLoggerTest, LogTotalDuration) {
  double duration = 3.0;
  _master.log_total_duration(duration);
  ASSERT_EQ(_logger->_durationTotal, duration);
  ASSERT_EQ(_logger2->_durationTotal, duration);
}

TEST_F(MasterLoggerTest, LogStoppingCriterion) {
  StoppingCriterion criterion = StoppingCriterion::absolute_gap;
  _master.log_stop_criterion_reached(criterion);
  ASSERT_EQ(_logger->_stopping_criterion, StoppingCriterion::absolute_gap);
  ASSERT_EQ(_logger2->_stopping_criterion, StoppingCriterion::absolute_gap);
}
