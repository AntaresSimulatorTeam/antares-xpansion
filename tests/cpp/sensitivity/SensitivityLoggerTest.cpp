#include "Commons.h"
#include "SensitivityFileLogger.h"
#include "SensitivityILogger.h"
#include "SensitivityMasterLogger.h"
#include "SensitivityLogger.h"
#include "gtest/gtest.h"

class SensitivityFileLoggerTest : public ::testing::Test {
 public:
  std::string _fileName;

  void SetUp() { _fileName = std::tmpnam(nullptr); }
  void TearDown() { std::remove(_fileName.c_str()); }
};

TEST_F(SensitivityFileLoggerTest, InvalidFileNotified) {
  const std::string& expectedErrorString =
      "Invalid file name passed as parameter";
  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());

  SensitivityFileLogger sensitivityFileLogger("");
  std::cerr.rdbuf(initialBufferCerr);

  ASSERT_TRUE(redirectedErrorStream.str().find(expectedErrorString) !=
              std::string::npos);
}

TEST_F(SensitivityFileLoggerTest, EmptyFileCreatedAtInit) {
  SensitivityFileLogger fileLog(_fileName);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;

  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();

  ASSERT_TRUE(fileStream.good() && stringStreamFromFile.str().empty());
}

class SensitivityUserLoggerTest : public ::testing::Test {
 public:
  const std::string indent_1 = "\t";

  SensitivityUserLoggerTest() : _logger(_stream) {}

  std::stringstream _stream;
  SensitivityLogger _logger;

  double epsilon = 12;
  double best_benders_cost = 1e6;

  const std::string semibase_name = "semibase";
  const std::string peak_name = "peak";

  const std::string str_pb_type = "my_pb_type";
};

TEST_F(SensitivityUserLoggerTest, EmptyStreamAtInit) {
  ASSERT_EQ(_stream.str().size(), 0);
}

TEST_F(SensitivityUserLoggerTest, InvalidStreamNotified) {
  const std::string expectedErrorString =
      "Invalid stream passed as parameter\n";

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  std::ofstream invalidStream("");

  SensitivityLogger userLogger(invalidStream);

  std::cerr.rdbuf(initialBufferCerr);

  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString);
}

TEST_F(SensitivityUserLoggerTest, DisplayMessage) {
  std::stringstream expected;
  expected << "Message" << std::endl;

  _logger.display_message("Message");
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, InitLog) {
  SensitivityOutputData output_data;
  output_data.best_benders_cost = best_benders_cost;
  output_data.epsilon = epsilon;

  std::stringstream expected;
  expected << std::endl;
  expected << "Best overall cost = "
           << xpansion::logger::commons::create_str_million_euros(
                  output_data.best_benders_cost)
           << MILLON_EUROS << std::endl;
  expected << "epsilon = " << output_data.epsilon << EUROS << std::endl;
  expected << std::endl;

  _logger.log_at_start(output_data);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogBeginPbResolution) {
  auto pb_data = SinglePbData(SensitivityPbType::CAPEX, str_pb_type,
                              "some_name", "my_opt_dir");
  std::stringstream expected;
  expected << "Solving " << pb_data.get_pb_description() << "..." << std::endl;

  _logger.log_begin_pb_resolution(pb_data);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSolutionProjection) {
  auto pb_data =
      SinglePbData(SensitivityPbType::PROJECTION, str_pb_type, "some_name",
                   "my_opt_dir", 10, 1e6, {{"peak", 15}, {"semibase", 50}}, 0);

  std::stringstream expected;
  expected << indent_1 << pb_data.get_pb_description() << " = "
           << pb_data.objective << MW << std::endl;
  expected << indent_1 << "Overall cost = "
           << xpansion::logger::commons::create_str_million_euros(
                  pb_data.system_cost)
           << MILLON_EUROS << std::endl;
  expected << std::endl;

  _logger.log_pb_solution(pb_data);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSolutionCapex) {
  auto pb_data =
      SinglePbData(SensitivityPbType::CAPEX, str_pb_type, "some_name",
                   "my_opt_dir", 10, 1e6, {{"peak", 15}, {"semibase", 50}}, 0);

  std::stringstream expected;
  expected << indent_1 << pb_data.get_pb_description() << " = "
           << xpansion::logger::commons::create_str_million_euros(
                  pb_data.objective)
           << MILLON_EUROS << std::endl;
  expected << indent_1 << "Overall cost = "
           << xpansion::logger::commons::create_str_million_euros(
                  pb_data.system_cost)
           << MILLON_EUROS << std::endl;
  expected << std::endl;

  _logger.log_pb_solution(pb_data);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogAtEnding) {
  std::stringstream expected;
  expected << "--- Sensitivity analysis completed ---" << std::endl;

  _logger.log_at_ending();
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSummaryOnlyCapex) {
  auto capex_min_data =
      SinglePbData(SensitivityPbType::CAPEX, str_pb_type, "", MIN_C, 1040, 1390,
                   {{peak_name, 14}, {semibase_name, 10}}, 42);

  auto capex_max_data =
      SinglePbData(SensitivityPbType::CAPEX, str_pb_type, "", MAX_C, 1224, 1490,
                   {{peak_name, 17}, {semibase_name, 11}}, 37);

  std::vector<SinglePbData> pbs_data = {capex_min_data, capex_max_data};
  auto output_data =
      SensitivityOutputData(epsilon, best_benders_cost, pbs_data);

  std::stringstream expected;
  expected << "Sensitivity analysis summary "
           << "(epsilon = " << output_data.epsilon << EUROS << "):" << std::endl
           << std::endl;

  expected << indent_1 << "CAPEX interval: ["
           << xpansion::logger::commons::create_str_million_euros(1040)
           << MILLON_EUROS << ", "
           << xpansion::logger::commons::create_str_million_euros(1224)
           << MILLON_EUROS << "]" << std::endl
           << std::endl;

  _logger.log_summary(output_data);
  ASSERT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSummaryOnlyProjection) {
  auto projection_min_peak =
      SinglePbData(SensitivityPbType::PROJECTION, str_pb_type, peak_name, MIN_C,
                   13, 1490, {{peak_name, 13}, {semibase_name, 11}}, 1);

  auto projection_max_peak =
      SinglePbData(SensitivityPbType::PROJECTION, str_pb_type, peak_name, MAX_C,
                   24, 1490, {{peak_name, 24}, {semibase_name, 10}}, 0);

  auto projection_min_semibase =
      SinglePbData(SensitivityPbType::PROJECTION, str_pb_type, semibase_name,
                   MIN_C, 10, 1390, {{peak_name, 14}, {semibase_name, 10}}, 2);

  auto projection_max_semibase =
      SinglePbData(SensitivityPbType::PROJECTION, str_pb_type, semibase_name,
                   MAX_C, 11, 1490, {{peak_name, 13}, {semibase_name, 11}}, 3);

  std::vector<SinglePbData> pbs_data = {
      projection_min_peak, projection_max_peak, projection_min_semibase,
      projection_max_semibase};

  auto output_data =
      SensitivityOutputData(epsilon, best_benders_cost, pbs_data);

  std::stringstream expected;
  expected << "Sensitivity analysis summary "
           << "(epsilon = " << output_data.epsilon << EUROS << "):" << std::endl
           << std::endl;

  expected << indent_1 << "Investment intervals of candidates:" << std::endl;
  expected << indent_1 << indent_1 << peak_name << ": [" << 13 << MW << ", "
           << 24 << MW << "]" << std::endl;

  expected << indent_1 << indent_1 << semibase_name << ": [" << 10 << MW << ", "
           << 11 << MW << "]" << std::endl;
  expected << std::endl;

  _logger.log_summary(output_data);
  ASSERT_EQ(_stream.str(), expected.str());
}

class SensitivityLoggerMock : public SensitivityILogger {
 public:
  SensitivityLoggerMock() {
    _initCall = false;
    _beginPbResolutionCall = false;
    _pbSolutionCall = false;
    _summaryCall = false;
    _endingCall = false;
  }

  bool _initCall;
  bool _beginPbResolutionCall;
  bool _pbSolutionCall;
  bool _summaryCall;
  bool _endingCall;

  std::string _displaymessage;

  void display_message(const std::string& str) override {
    _displaymessage = str;
  }

  void log_at_start(const SensitivityOutputData& output_data) override {
    _initCall = true;
  }

  void log_begin_pb_resolution(const SinglePbData& pb_data) override {
    _beginPbResolutionCall = true;
  }

  void log_pb_solution(const SinglePbData& pb_data) override {
    _pbSolutionCall = true;
  }

  void log_summary(const SensitivityOutputData& output_data) override {
    _summaryCall = true;
  }

  void log_at_ending() override { _endingCall = true; }
};

class SensitivityMasterLoggerTest : public ::testing::Test {
 public:
  SensitivityMasterLoggerTest() {
    _logger = std::make_shared<SensitivityLoggerMock>();
    _logger2 = std::make_shared<SensitivityLoggerMock>();
    _master.addLogger(_logger);
    _master.addLogger(_logger2);
  }
  SensitivityMasterLogger _master;
  std::shared_ptr<SensitivityLoggerMock> _logger;
  std::shared_ptr<SensitivityLoggerMock> _logger2;
};

TEST_F(SensitivityMasterLoggerTest, DisplayMessage) {
  std::string message = "message";
  _master.display_message(message);
  ASSERT_EQ(_logger->_displaymessage, message);
  ASSERT_EQ(_logger2->_displaymessage, message);
}

TEST_F(SensitivityMasterLoggerTest, InitLog) {
  SensitivityOutputData output_data;
  _master.log_at_start(output_data);
  ASSERT_TRUE(_logger->_initCall);
  ASSERT_TRUE(_logger2->_initCall);
}

TEST_F(SensitivityMasterLoggerTest, BeginPbResolutionLog) {
  SinglePbData pb_data;
  _master.log_begin_pb_resolution(pb_data);
  ASSERT_TRUE(_logger->_beginPbResolutionCall);
  ASSERT_TRUE(_logger2->_beginPbResolutionCall);
}

TEST_F(SensitivityMasterLoggerTest, PbSolutionLog) {
  SinglePbData pb_data;
  _master.log_pb_solution(pb_data);
  ASSERT_TRUE(_logger->_pbSolutionCall);
  ASSERT_TRUE(_logger2->_pbSolutionCall);
}

TEST_F(SensitivityMasterLoggerTest, SummaryLog) {
  SensitivityOutputData output_data;
  _master.log_summary(output_data);
  ASSERT_TRUE(_logger->_summaryCall);
  ASSERT_TRUE(_logger2->_summaryCall);
}

TEST_F(SensitivityMasterLoggerTest, EndingLog) {
  _master.log_at_ending();
  ASSERT_TRUE(_logger->_endingCall);
  ASSERT_TRUE(_logger2->_endingCall);
}