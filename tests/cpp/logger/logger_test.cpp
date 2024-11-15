#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>

#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "LogPrefixManip.h"
#include "RandomDirGenerator.h"
#include "gtest/gtest.h"
#include "antares-xpansion/benders/logger/Master.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/benders/logger/UserFile.h"

using namespace xpansion::logger;

void addCandidate(LogData& logData, std::string candidate, double invest,
                  double minInvest, double maxInvest) {
  logData.x_cut[candidate] = invest;
  logData.min_invest[candidate] = minInvest;
  logData.max_invest[candidate] = maxInvest;
}

const LogPoint x_in = {{"candidate1", 52}, {"candidate2", 4}};
const LogPoint x_out = {{"candidate1", 56}, {"candidate2", 6}};
const LogPoint x_cut = {{"candidate1", 60}, {"candidate2", 12}};
const LogPoint min_invest = {{"candidate1", 5}, {"candidate2", 2}};
const LogPoint max_invest = {{"candidate1", 10}, {"candidate2", 20}};
const LogData best_iteration_data = {
    15e5,  255e6,      200e6,      63,  63,    587e8, 8562e8, x_in, x_out,
    x_cut, min_invest, max_invest, 9e9, 3e-15, 5876,  999,    898,  23};
const LogData last_iteration_data = {
    1e5,   255e6,      200e6,      159, 63,    587e8, 8562e8, x_in, x_out,
    x_cut, min_invest, max_invest, 9e9, 3e-15, 5876,  9999,   898,  25};

class FileLoggerTest : public ::testing::Test {
 public:
  void SetUp() { _fileName = std::tmpnam(nullptr); }

  void TearDown() { std::remove(_fileName.string().c_str()); }

  std::filesystem::path _fileName;
};

TEST_F(FileLoggerTest, InvalidFileNotified) {
  const std::string& expectedErrorString =
      "Invalid file name passed as parameter";
  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
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

TEST_F(FileLoggerTest, ShouldPrintRestartMessage) {
  std::stringstream expected_msg;
  expected_msg << " Restart Study..." << std::endl;
  UserFile fileLog(_fileName);
  fileLog.display_restart_message();

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;
  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();
  auto stringFromFileWithoutPrefix =
      RemovePrefixFromLineMessage(stringStreamFromFile.str());
  ASSERT_EQ(stringFromFileWithoutPrefix, expected_msg.str());
}
TEST_F(FileLoggerTest, ShouldPrintRestartElapsedTimeMessage) {
  std::stringstream expected_msg;
  // 2 Days 10 hours 45 minutes 9 seconds
  const int days(2);
  const int hours(10);
  const int minutes(45);
  const int seconds(9);

  const double elapsed_time_numeric(days * 24 * 60 * 60 + hours * 60 * 60 +
                                    minutes * 60 + seconds);
  const std::string elapsed_time_str(std::to_string(days) + " days " +
                                     std::to_string(hours) + " hours " +
                                     std::to_string(minutes) + " minutes " +
                                     std::to_string(seconds) + " seconds ");
  expected_msg << " \tElapsed time: " << elapsed_time_str << std::endl;
  UserFile fileLog(_fileName);
  fileLog.restart_elapsed_time(elapsed_time_numeric);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;
  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();
  auto stringFromFileWithoutPrefix =
      RemovePrefixFromLineMessage(stringStreamFromFile.str());

  ASSERT_EQ(stringFromFileWithoutPrefix, expected_msg.str());
}
TEST_F(FileLoggerTest, ShouldPrintNumberOfIterationsBeforeRestart) {
  std::stringstream expected_msg;

  const int num_iteration(684);
  expected_msg << " \tNumber of Iterations performed: " << num_iteration
               << std::endl;
  UserFile fileLog(_fileName);
  fileLog.number_of_iterations_before_restart(num_iteration);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;
  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();
  auto stringFromFileWithoutPrefix =
      RemovePrefixFromLineMessage(stringStreamFromFile.str());

  ASSERT_EQ(stringFromFileWithoutPrefix, expected_msg.str());
}
TEST_F(FileLoggerTest, ShouldPrintBestIterationsBeforeRestart) {
  std::stringstream expected_msg;

  const int best_it(84);
  expected_msg << " \tBest Iteration: " << best_it << std::endl;
  UserFile fileLog(_fileName);
  fileLog.restart_best_iteration(best_it);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;
  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();
  auto stringFromFileWithoutPrefix =
      RemovePrefixFromLineMessage(stringStreamFromFile.str());

  ASSERT_EQ(stringFromFileWithoutPrefix, expected_msg.str());
}

TEST_F(FileLoggerTest, ShouldPrintBestIterationsInfos) {
  std::stringstream expected_msg;

  const auto indent_1("\t");
  const std::string indent_0 = "\t\t";
  LogData l;
  l.master_time = 898;
  l.subproblem_time = 6;
  l.invest_cost = 20e6;
  l.best_ub = 3e6;
  l.lb = 2e6;
  l.subproblem_cost = 15.5e6;

  addCandidate(l, "candidate1", 58.0, 23.0, 64.0);
  addCandidate(l, "candidate2", 8.0, 3.0, 40.0);
  expected_msg
      << " \tBest Iteration Infos: " << std::endl
      << " \tMaster solved in " << l.master_time << " s" << std::endl
      << " " << indent_0 << "Candidates:" << std::endl
      << " " << indent_0 << indent_1
      << "candidate1 = 58.00 invested MW -- possible interval [23.00; 64.00] MW"
      << std::endl
      << " " << indent_0 << indent_1
      << "candidate2 =  8.00 invested MW -- possible interval [ 3.00; 40.00] MW"
      << std::endl
      << " " << indent_1
      << "Subproblems solved in (walltime): " << l.subproblem_time << " s"
      << std::endl
      << " " << indent_0 << "Solution =" << std::endl
      << " " << indent_0 << indent_1 << "Operational cost =       15.50 Me"
      << std::endl
      << " " << indent_0 << indent_1 << " Investment cost =       20.00 Me"
      << std::endl
      << " " << indent_0 << indent_1 << "    Overall cost =       35.50 Me"
      << std::endl
      << " " << indent_0 << indent_1 << "   Best Solution =        3.00 Me"
      << std::endl
      << " " << indent_0 << indent_1 << "     Lower Bound =        2.00 Me"
      << std::endl
      << " " << indent_0 << indent_1 << "    Absolute gap = 1.00000e+06 e"
      << std::endl
      << " " << indent_0 << indent_1 << "    Relative gap = 3.33333e-01"
      << std::endl;
  UserFile fileLog(_fileName);
  fileLog.restart_best_iterations_infos(l);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;
  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();

  auto stringFromFileWithoutPrefix =
      RemovePrefixFromMessage(stringStreamFromFile);

  ASSERT_EQ(stringFromFileWithoutPrefix, expected_msg.str());
}

class UserLoggerTest : public ::testing::Test {
 public:
  const std::string indent_0 = "\t\t";
  const std::string indent_1 = "\t";

  UserLoggerTest() : _logger(_stream) {}

  std::stringstream _stream;
  User _logger;
};

TEST_F(UserLoggerTest, EmptyStreamAtInit) {
  ASSERT_EQ(_stream.str().size(), 0);
}

TEST_F(UserLoggerTest, InvalidStreamNotified) {
  const std::string expected_msg = " Invalid stream passed as parameter\n";

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  std::ofstream invalidStream("");

  User userLogger(invalidStream);

  std::cerr.rdbuf(initialBufferCerr);

  auto logWithoutPrefix =
      RemovePrefixFromLineMessage(redirectedErrorStream.str());

  ASSERT_EQ(logWithoutPrefix, expected_msg);
}

TEST_F(UserLoggerTest, InitLog) {
  LogData logData;
  logData.it = 1;
  std::stringstream expected;
  expected << " ITERATION 1:" << std::endl;

  _logger.log_at_initialization(logData.it);

  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, IterationStartLogCandidateOrder) {
  LogData logData;
  addCandidate(logData, "z", 50.0, 0.0, 100.0);
  addCandidate(logData, "a", 10.0, 0.0, 50.0);
  addCandidate(logData, "c", 50.0, 0.0, 100.0);

  std::stringstream expected;
  expected << " " << indent_0 << "Candidates:" << std::endl;
  expected << " " << indent_0 << indent_1
           << "a = 10.00 invested MW -- possible interval [0.00;  50.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "c = 50.00 invested MW -- possible interval [0.00; 100.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "z = 50.00 invested MW -- possible interval [0.00; 100.00] MW"
           << std::endl;

  _logger.log_iteration_candidates(logData);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, IterationStartLogCandidateLongInvestment) {
  LogData logData;
  addCandidate(logData, "z", 5000000.0, 0.0, 10000000.0);
  addCandidate(logData, "a", 10.0, 200.0, 50.0);
  addCandidate(logData, "b", 20.0, 0.0, 200.0);

  std::stringstream expected;
  expected << " " << indent_0 << "Candidates:" << std::endl;
  expected << " " << indent_0 << indent_1
           << "a =      10.00 invested MW -- possible interval [200.00;       "
              "50.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "b =      20.00 invested MW -- possible interval [  0.00;      "
              "200.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "z = 5000000.00 invested MW -- possible interval [  0.00; "
              "10000000.00] MW"
           << std::endl;

  _logger.log_iteration_candidates(logData);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, IterationStartLogCandidateNameLength) {
  LogData logData;
  addCandidate(logData, "z", 50.0, 0.0, 100.0);
  addCandidate(logData, "a", 10.0, 0.0, 50.0);
  addCandidate(logData, "very long name of investment", 50.0, 0.0, 100.0);

  std::stringstream expected;
  expected << " " << indent_0 << "Candidates:" << std::endl;
  expected << " " << indent_0 << indent_1
           << "                           a = 10.00 invested MW -- possible "
              "interval [0.00;  50.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "very long name of investment = 50.00 invested MW -- possible "
              "interval [0.00; 100.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "                           z = 50.00 invested MW -- possible "
              "interval [0.00; 100.00] MW"
           << std::endl;

  _logger.log_iteration_candidates(logData);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, IterationEndLog) {
  LogData logData;
  logData.subproblem_cost = 15.5e6;
  logData.invest_cost = 20e6;
  logData.best_ub = 3e6;
  logData.lb = 2e6;

  std::stringstream expected;
  expected << " " << indent_0 << "Solution =" << std::endl;
  expected << " " << indent_0 << indent_1 << "Operational cost =       15.50 Me"
           << std::endl;
  expected << " " << indent_0 << indent_1 << " Investment cost =       20.00 Me"
           << std::endl;
  expected << " " << indent_0 << indent_1 << "    Overall cost =       35.50 Me"
           << std::endl;
  expected << " " << indent_0 << indent_1 << "   Best Solution =        3.00 Me"
           << std::endl;
  expected << " " << indent_0 << indent_1 << "     Lower Bound =        2.00 Me"
           << std::endl;
  expected << " " << indent_0 << indent_1 << "    Absolute gap = 1.00000e+06 e"
           << std::endl;
  expected << " " << indent_0 << indent_1 << "    Relative gap = 3.33333e-01"
           << std::endl;

  _logger.log_at_iteration_end(logData);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, IterationEndLogLongCost) {
  LogData logData;
  logData.subproblem_cost = 150000000.5e6;
  logData.invest_cost = 200000000e6;
  logData.best_ub = 100e6;
  logData.lb = 3e6;

  std::stringstream expected;
  expected << " " << indent_0 << "Solution =" << std::endl;
  expected << " " << indent_0 << indent_1
           << "Operational cost = 150000000.50 Me" << std::endl;
  expected << " " << indent_0 << indent_1
           << " Investment cost = 200000000.00 Me" << std::endl;
  expected << " " << indent_0 << indent_1
           << "    Overall cost = 350000000.50 Me" << std::endl;
  expected << " " << indent_0 << indent_1
           << "   Best Solution =       100.00 Me" << std::endl;
  expected << " " << indent_0 << indent_1
           << "     Lower Bound =         3.00 Me" << std::endl;
  expected << " " << indent_0 << indent_1 << "    Absolute gap =  9.70000e+07 e"
           << std::endl;
  expected << " " << indent_0 << indent_1 << "    Relative gap =  9.70000e-01"
           << std::endl;

  _logger.log_at_iteration_end(logData);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, MaxIterationsReached) {
  std::stringstream expected;
  expected << " --- Run completed: maximum iterations reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::max_iteration;
  _logger.log_stop_criterion_reached(criterion);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, TimeLimitReached) {
  std::stringstream expected;
  expected << " --- Run completed: timelimit reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::timelimit;
  _logger.log_stop_criterion_reached(criterion);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, AbsoluteGapReached) {
  std::stringstream expected;
  expected << " --- Run completed: absolute gap reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::absolute_gap;
  _logger.log_stop_criterion_reached(criterion);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, RelativeGapReached) {
  std::stringstream expected;
  expected << " --- Run completed: relative gap reached" << std::endl;
  StoppingCriterion criterion = StoppingCriterion::relative_gap;
  _logger.log_stop_criterion_reached(criterion);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, EndLog) {
  LogData logData;
  logData.best_it = 1;
  logData.subproblem_cost = 1e6;
  logData.invest_cost = 10e6;
  std::stringstream expected;
  expected << " " << indent_1
           << "Total number of iterations done = " << logData.it << std::endl;
  expected << " " << indent_1 << "Best solution = it 1" << std::endl;
  expected << " " << indent_1 << " Overall cost = 11.00 Me" << std::endl;

  _logger.log_at_ending(logData);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, CumulativeNumberOfSubProblemResolved) {
  auto number(9150);
  _logger.cumulative_number_of_sub_problem_solved(number);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);
  std::stringstream expected;
  expected << " " << indent_1
           << "cumulative number of subproblem resolutions: " << number
           << std::endl;
  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, DifferentCallsAddToTheSameStream) {
  LogData logData1;
  logData1.it = 1;
  addCandidate(logData1, "z", 5000000.0, 0.0, 10000000.0);
  addCandidate(logData1, "a", 10.0, 200.0, 50.0);

  std::stringstream expected;
  expected << " " << indent_0 << "Candidates:" << std::endl;
  expected << " " << indent_0 << indent_1
           << "a =      10.00 invested MW -- possible interval [200.00;       "
              "50.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "z = 5000000.00 invested MW -- possible interval [  0.00; "
              "10000000.00] MW"
           << std::endl;

  LogData logData2;
  logData2.it = 2;
  addCandidate(logData2, "b", 6000000.0, 0.0, 10000000.0);
  addCandidate(logData2, "a", 200.0, 200.0, 50.0);

  expected << " " << indent_0 << "Candidates:" << std::endl;
  expected << " " << indent_0 << indent_1
           << "a =     200.00 invested MW -- possible interval [200.00;       "
              "50.00] MW"
           << std::endl;
  expected << " " << indent_0 << indent_1
           << "b = 6000000.00 invested MW -- possible interval [  0.00; "
              "10000000.00] MW"
           << std::endl;
  _logger.log_iteration_candidates(logData1);
  _logger.log_iteration_candidates(logData2);
  auto logWithoutPrefix = RemovePrefixFromMessage(_stream);

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, DisplayMessage) {
  std::stringstream expected;
  expected << " Message" << std::endl;

  _logger.display_message("Message");
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, LogMasterDuration) {
  std::stringstream expected;
  expected << " " << indent_1 << "Master solved in 3 s" << std::endl;
  _logger.log_master_solving_duration(3.000000);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, LogSubProblemDuration) {
  std::stringstream expected;
  expected << " " << indent_1 << "Subproblems solved in (walltime): 3 s"
           << std::endl;
  _logger.LogSubproblemsSolvingWalltime(3.000000);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, LogTotalDuration) {
  std::stringstream expected;
  expected << " Benders ran in 3 s" << std::endl;
  _logger.log_total_duration(3.000000);
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, LogAtInitialRelaxation) {
  std::stringstream expected;
  expected << " --- Switch master formulation to relaxed" << std::endl;
  _logger.LogAtInitialRelaxation();
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

TEST_F(UserLoggerTest, LogAtSwitchInteger) {
  std::stringstream expected;
  expected << " --- Relaxed gap reached, switch master formulation to integer"
           << std::endl;
  _logger.LogAtSwitchToInteger();
  auto logWithoutPrefix = RemovePrefixFromLineMessage(_stream.str());

  ASSERT_EQ(logWithoutPrefix, expected.str());
}

class SimpleLoggerMock : public ILogger {
 public:
  SimpleLoggerMock() {
    _initCall = false;
    _iterationStartCall = false;
    _iterationEndCall = false;
    _endingCall = false;
    _switchToIntegerCall = false;
    _initialRelaxationCall = false;

    _stopping_criterion = StoppingCriterion::empty;

    _durationMaster = 0.0;
    _durationSubproblem = 0.0;
    _durationTotal = 0.0;
  }

  void display_message(const std::string& str) { _displaymessage = str; }
  void display_message(const std::string& str, LogUtils::LOGLEVEL level,
                       const std::string& context) {
    _displaymessage = str;
  }

  void PrintIterationSeparatorBegin() override {
    //
  }
  void PrintIterationSeparatorEnd() override {
    //
  }

  void log_at_initialization(const int it_number) override { _initCall = true; }

  void log_iteration_candidates(const LogData& d) override {
    _iterationStartCall = true;
  }

  void log_master_solving_duration(double durationInSeconds) {
    _durationMaster = durationInSeconds;
  }

  void LogSubproblemsSolvingWalltime(double durationInSeconds) {
    _durationSubproblem = durationInSeconds;
  }

  void LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds) {
    //
  }

  void log_at_iteration_end(const LogData& d) override {
    _iterationEndCall = true;
  }

  void log_at_ending(const LogData& d) override { _endingCall = true; }
  void log_total_duration(double durationInSeconds) {
    _durationTotal = durationInSeconds;
  }

  void log_stop_criterion_reached(
      const StoppingCriterion stopping_criterion) override {
    _stopping_criterion = stopping_criterion;
  }
  void display_restart_message() override {
    //
  }
  void restart_elapsed_time(const double elapsed_time) override {
    //
  }
  void number_of_iterations_before_restart(const int num_iterations) override {
    //
  }
  void restart_best_iteration(const int best_iterations) override {  //
  }
  void restart_best_iterations_infos(
      const LogData& best_iterations_data) override {
    //
  }

  void LogAtInitialRelaxation() { _initialRelaxationCall = true; }

  void LogAtSwitchToInteger() { _switchToIntegerCall = true; }
  void cumulative_number_of_sub_problem_solved(int number) {
    _cumulativeNumberOfSubProblemResolved = true;
  }

  bool _initCall;
  bool _iterationStartCall;
  bool _iterationEndCall;
  bool _endingCall;
  bool _switchToIntegerCall;
  bool _initialRelaxationCall;
  bool _cumulativeNumberOfSubProblemResolved;
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
  _master.log_at_initialization(logData.it);
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

TEST_F(MasterLoggerTest, CumulativeNumberOfSubProblemResolved) {
  LogData logData;
  _master.cumulative_number_of_sub_problem_solved(39);
  ASSERT_TRUE(_logger->_cumulativeNumberOfSubProblemResolved);
  ASSERT_TRUE(_logger2->_cumulativeNumberOfSubProblemResolved);
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
  _master.LogSubproblemsSolvingWalltime(duration);
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

TEST_F(MasterLoggerTest, LogAtInitialRelaxation) {
  _master.LogAtInitialRelaxation();
  ASSERT_TRUE(_logger->_initialRelaxationCall);
  ASSERT_TRUE(_logger2->_initialRelaxationCall);
}

TEST_F(MasterLoggerTest, LogSwitchToInteger) {
  _master.LogAtSwitchToInteger();
  ASSERT_TRUE(_logger->_switchToIntegerCall);
  ASSERT_TRUE(_logger2->_switchToIntegerCall);
}
static constexpr const char* const DELIMITER = "\t";

TEST(LogDestinationTest, WithInvalidEmptyFilePath) {
  const std::filesystem::path invalid_file_path("");
  std::ostringstream expected_msg;
  expected_msg << " Could not open the file: "
               << std::quoted(invalid_file_path.string().c_str()) << "\n";

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());

  LogDestination log_dest(invalid_file_path);

  std::cerr.rdbuf(initialBufferCerr);
  auto err_str = RemovePrefixFromMessage(redirectedErrorStream);
  ASSERT_EQ(expected_msg.str(), err_str);
}

TEST(LogDestinationTest, StdoutWithAValidMessage) {
  const std::string msg = "Hello!";
  std::streamsize indentation = 25;
  const std::string expected_msg =
      msg + std::string((size_t)indentation - msg.size(), ' ');

  std::stringstream redirectedStdout;
  std::streambuf* initialBufferCout = std::cout.rdbuf(redirectedStdout.rdbuf());

  LogDestination log_dest(indentation);
  log_dest << msg;

  std::cout.rdbuf(initialBufferCout);

  ASSERT_EQ(expected_msg, redirectedStdout.str());
}

std::string FileContent(const std::filesystem::path& file) {
  std::ifstream file_stream(file);

  std::string content((std::istreambuf_iterator<char>(file_stream)),
                      (std::istreambuf_iterator<char>()));
  return content;
}

TEST(LogDestinationTest, MessageWithAValidFile) {
  const std::string msg = "Hello!";
  std::streamsize indentation = 40;
  const std::string expected_msg =
      msg + std::string((size_t)indentation - msg.size(), ' ');

  auto log_file =
      CreateRandomSubDir(std::filesystem::temp_directory_path()) / "log.txt";
  LogDestination log_dest(log_file);
  log_dest << msg;
  ASSERT_TRUE(std::filesystem::exists(log_file));

  ASSERT_EQ(expected_msg, FileContent(log_file));
}

TEST(MathLoggerHeadersManagerTest, LongBenders) {
  HEADERSTYPE headers_type = HEADERSTYPE::LONG;
  HeadersManager headers_manager(headers_type, BENDERSMETHOD::BENDERS);

  std::vector<std::string> expected_headers = {"Ite",
                                               "Lb",
                                               "Ub",
                                               "BestUb",
                                               "AbsGap",
                                               "RelGap",
                                               "MinSpx",
                                               "MaxSpx",
                                               "NbSubPbSolv",
                                               "CumulNbSubPbSolv",
                                               "IteTime (s)",
                                               "MasterTime (s)",
                                               "SPWallTime (s)",
                                               "SPCpuTime (s)",
                                               "NotSolvingWallTime (s)"};
  ASSERT_EQ(expected_headers, headers_manager.HeadersList());
}

TEST(MathLoggerHeadersManagerTest, ShortBenders) {
  HEADERSTYPE headers_type = HEADERSTYPE::SHORT;
  HeadersManager headers_manager(headers_type, BENDERSMETHOD::BENDERS);

  std::vector<std::string> expected_headers = {
      "Ite",    "Lb",     "Ub",      "BestUb",     "AbsGap",    "RelGap",
      "MinSpx", "MaxSpx", "IteTime (s)", "MasterTime (s)", "SPWallTime (s)"};
  ASSERT_EQ(expected_headers, headers_manager.HeadersList());
}

TEST(MathLoggerHeadersManagerTest, LongBendersByBatch) {
  HEADERSTYPE headers_type = HEADERSTYPE::LONG;
  HeadersManager headers_manager(headers_type, BENDERSMETHOD::BENDERS_BY_BATCH);

  std::vector<std::string> expected_headers = {"Ite",
                                               "Lb",
                                               "MinSpx",
                                               "MaxSpx",
                                               "NbSubPbSolv",
                                               "CumulNbSubPbSolv",
                                               "IteTime (s)",
                                               "MasterTime (s)",
                                               "SPWallTime (s)",
                                               "SPCpuTime (s)",
                                               "NotSolvingWallTime (s)"};
  ASSERT_EQ(expected_headers, headers_manager.HeadersList());
}

TEST(MathLoggerHeadersManagerTest, ShortBendersByBatch) {
  HEADERSTYPE headers_type = HEADERSTYPE::SHORT;
  HeadersManager headers_manager(headers_type, BENDERSMETHOD::BENDERS_BY_BATCH);

  std::vector<std::string> expected_headers = {
      "Ite",         "Lb",      "MinSpx",     "MaxSpx",
      "NbSubPbSolv", "IteTime (s)", "MasterTime (s)", "SPWallTime (s)"};
  ASSERT_EQ(expected_headers, headers_manager.HeadersList());
}

TEST(MathLoggerBendersByBatchTest, HeadersListStdOutShort) {
  HEADERSTYPE headers_type = HEADERSTYPE::SHORT;
  HeadersManager headers_manager(headers_type, BENDERSMETHOD::BENDERS_BY_BATCH);
  std::streamsize width = 25;

  std::ostringstream expected_msg;
  expected_msg << DELIMITER;
  for (const auto& header : headers_manager.HeadersList()) {
    expected_msg << std::setw(width) << std::left << header;
    expected_msg << DELIMITER;
  }
  expected_msg << std::endl;
  std::stringstream redirectedStdout;
  std::streambuf* initialBufferCout = std::cout.rdbuf(redirectedStdout.rdbuf());
  MathLoggerBendersByBatch benders_batch_logger(width);
  benders_batch_logger.write_header();
  std::cout.rdbuf(initialBufferCout);

  ASSERT_EQ(expected_msg.str(), redirectedStdout.str());
}

TEST(MathLoggerBendersByBatchTest, HeadersListFileLong) {
  HEADERSTYPE headers_type = HEADERSTYPE::LONG;
  HeadersManager headers_manager(headers_type, BENDERSMETHOD::BENDERS_BY_BATCH);
  std::streamsize width = 25;

  std::ostringstream expected_msg;
  expected_msg << DELIMITER;
  for (const auto& header : headers_manager.HeadersList()) {
    expected_msg << std::setw(width) << std::left << header;
    expected_msg << DELIMITER;
  }
  expected_msg << std::endl;
  auto log_file =
      CreateRandomSubDir(std::filesystem::temp_directory_path()) / "log.txt";
  MathLoggerBendersByBatch benders_batch_logger(log_file, width, headers_type);
  benders_batch_logger.write_header();

  ASSERT_EQ(expected_msg.str(), FileContent(log_file));
}

TEST(MathLoggerBendersByBatchTest, DataInFileLong) {
  HEADERSTYPE headers_type = HEADERSTYPE::LONG;
  std::streamsize width = 25;

  CurrentIterationData data;
  data.it = 35;
  data.lb = 256999;
  // data.ub = 222256999;
  // data.best_ub = 222256999;
  data.min_simplexiter = 3;
  data.max_simplexiter = 30;
  data.number_of_subproblem_solved = 657;
  data.cumulative_number_of_subproblem_solved = 1387;
  data.iteration_time = 1000;
  data.timer_master = 10;
  data.subproblems_walltime = 16;
  data.subproblems_cumulative_cputime = 160;
  auto time_not_solving =
      data.iteration_time - data.timer_master - data.subproblems_walltime;

  std::ostringstream expected_msg;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.it;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.lb;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.min_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.max_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width)
               << data.number_of_subproblem_solved;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width)
               << data.cumulative_number_of_subproblem_solved;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.iteration_time;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.timer_master;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.subproblems_walltime;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.subproblems_cumulative_cputime;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << time_not_solving;
  expected_msg << DELIMITER;
  expected_msg << std::endl;
  auto log_file =
      CreateRandomSubDir(std::filesystem::temp_directory_path()) / "log.txt";
  MathLoggerBendersByBatch benders_batch_logger(log_file, width, headers_type);
  benders_batch_logger.Print(data);

  ASSERT_EQ(expected_msg.str(), FileContent(log_file));
}

TEST(MathLoggerBendersByBatchTest, DataInStdOutShort) {
  HEADERSTYPE headers_type = HEADERSTYPE::SHORT;
  std::streamsize width = 25;

  CurrentIterationData data;
  data.it = 35;
  data.lb = 256999;
  // data.ub = 222256999;
  // data.best_ub = 222256999;
  data.min_simplexiter = 3;
  data.max_simplexiter = 30;
  data.number_of_subproblem_solved = 657;
  data.cumulative_number_of_subproblem_solved = 1387;
  data.iteration_time = 1000;
  data.timer_master = 10;
  data.subproblems_walltime = 16;
  data.subproblems_cumulative_cputime = 160;
  auto time_not_solving =
      data.iteration_time - data.timer_master - data.subproblems_walltime;

  std::ostringstream expected_msg;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.it;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.lb;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.min_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.max_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width)
               << data.number_of_subproblem_solved;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.iteration_time;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.timer_master;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.subproblems_walltime;
  expected_msg << DELIMITER;

  expected_msg << std::endl;
  std::stringstream redirectedStdout;
  std::streambuf* initialBufferCout = std::cout.rdbuf(redirectedStdout.rdbuf());
  MathLoggerBendersByBatch benders_batch_logger(width);
  benders_batch_logger.Print(data);
  std::cout.rdbuf(initialBufferCout);

  ASSERT_EQ(expected_msg.str(), redirectedStdout.str());
}

TEST(MathLoggerBendersBaseTest, DataInFileLong) {
  HEADERSTYPE headers_type = HEADERSTYPE::LONG;
  std::streamsize width = 25;

  CurrentIterationData data;
  data.it = 35;
  data.lb = 256999;
  data.ub = 222256999;
  data.best_ub = 22552256999;
  data.min_simplexiter = 3;
  data.max_simplexiter = 30;
  data.number_of_subproblem_solved = 657;
  data.cumulative_number_of_subproblem_solved = 1387;
  data.iteration_time = 1000;
  data.timer_master = 10;
  data.subproblems_walltime = 16;
  data.subproblems_cumulative_cputime = 160;
  auto time_not_solving =
      data.iteration_time - data.timer_master - data.subproblems_walltime;

  std::ostringstream expected_msg;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.it;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.lb;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.ub;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.best_ub;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(2) << data.best_ub - data.lb;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(2)
               << (data.best_ub - data.lb) / data.best_ub;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.min_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.max_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width)
               << data.number_of_subproblem_solved;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width)
               << data.cumulative_number_of_subproblem_solved;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.iteration_time;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.timer_master;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.subproblems_walltime;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.subproblems_cumulative_cputime;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << time_not_solving;
  expected_msg << DELIMITER;
  expected_msg << std::endl;
  auto log_file =
      CreateRandomSubDir(std::filesystem::temp_directory_path()) / "log.txt";
  MathLoggerBase benders_batch_logger(log_file, width, headers_type);
  benders_batch_logger.Print(data);

  ASSERT_EQ(expected_msg.str(), FileContent(log_file));
}

TEST(MathLoggerBendersBaseTest, DataInStdOutShort) {
  HEADERSTYPE headers_type = HEADERSTYPE::SHORT;
  std::streamsize width = 25;

  CurrentIterationData data;
  data.it = 35;
  data.lb = 256999;
  data.ub = 2222569996;
  data.best_ub = 22225556999;
  data.min_simplexiter = 3;
  data.max_simplexiter = 30;
  data.number_of_subproblem_solved = 657;
  data.cumulative_number_of_subproblem_solved = 1387;
  data.iteration_time = 1000;
  data.timer_master = 10;
  data.subproblems_walltime = 16;
  data.subproblems_cumulative_cputime = 160;
  auto time_not_solving =
      data.iteration_time - data.timer_master - data.subproblems_walltime;

  std::ostringstream expected_msg;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.it;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.lb;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.ub;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(10) << data.best_ub;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(2) << data.best_ub - data.lb;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::scientific
               << std::setprecision(2)
               << (data.best_ub - data.lb) / data.best_ub;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width) << data.min_simplexiter;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << data.max_simplexiter;
  expected_msg << DELIMITER;

  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.iteration_time;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.timer_master;
  expected_msg << DELIMITER;
  expected_msg << std::left << std::setw(width) << std::setprecision(2)
               << data.subproblems_walltime;
  expected_msg << DELIMITER;

  expected_msg << std::endl;
  std::stringstream redirectedStdout;
  std::streambuf* initialBufferCout = std::cout.rdbuf(redirectedStdout.rdbuf());
  MathLoggerBase benders_batch_logger(width);
  benders_batch_logger.Print(data);
  std::cout.rdbuf(initialBufferCout);

  ASSERT_EQ(expected_msg.str(), redirectedStdout.str());
}
