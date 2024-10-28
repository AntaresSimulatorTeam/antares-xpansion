#include "antares-xpansion/benders/logger/Commons.h"
#include "antares-xpansion/sensitivity/SensitivityFileLogger.h"
#include "antares-xpansion/sensitivity/SensitivityILogger.h"
#include "antares-xpansion/sensitivity/SensitivityLogger.h"
#include "antares-xpansion/sensitivity/SensitivityMasterLogger.h"
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

  EXPECT_TRUE(redirectedErrorStream.str().find(expectedErrorString) !=
              std::string::npos);
}

TEST_F(SensitivityFileLoggerTest, EmptyFileCreatedAtInit) {
  SensitivityFileLogger fileLog(_fileName);

  std::ifstream fileStream(_fileName);
  std::stringstream stringStreamFromFile;

  stringStreamFromFile << fileStream.rdbuf();
  fileStream.close();

  EXPECT_TRUE(fileStream.good() && stringStreamFromFile.str().empty());
}

class SensitivityUserLoggerTest : public ::testing::Test {
 public:
  const std::string indent_1 = "\t";

  SensitivityUserLoggerTest() : _logger(_stream) {}

  std::stringstream _stream;
  SensitivityLogger _logger;

  double epsilon = 12;
  double best_benders_cost = 1e6;
  double benders_capex = 1e5;

  const std::string semibase_name = "semibase";
  const std::string peak_name = "peak";

  const std::string str_pb_type = "my_pb_type";
};

TEST_F(SensitivityUserLoggerTest, EmptyStreamAtInit) {
  EXPECT_EQ(_stream.str().size(), 0);
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

  EXPECT_EQ(redirectedErrorStream.str(), expectedErrorString);
}

TEST_F(SensitivityUserLoggerTest, DisplayMessage) {
  std::stringstream expected;
  expected << "Message" << std::endl << std::endl;

  _logger.display_message("Message");
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, InitLog) {
  SensitivityInputData input_data;
  input_data.best_ub = best_benders_cost;
  input_data.epsilon = epsilon;
  input_data.benders_capex = benders_capex;
  input_data.benders_solution = {{peak_name, 15}, {semibase_name, 60.3}};

  std::stringstream expected;
  expected << std::endl;
  expected << "--- Recall of the best investment solution ---" << std::endl
           << std::endl;
  expected << indent_1 << "Best overall cost = "
           << xpansion::logger::commons::create_str_million_euros(
                  input_data.best_ub)
           << MILLON_EUROS << std::endl
           << std::endl;
  expected << indent_1 << "Best capex = "
           << xpansion::logger::commons::create_str_million_euros(benders_capex)
           << MILLON_EUROS << std::endl
           << std::endl;
  expected << indent_1 << "Best investment solution = " << std::endl;
  for (const auto& kvp : input_data.benders_solution) {
    expected << indent_1 << indent_1 << kvp.first << ": "
             << xpansion::logger::commons::create_str_mw(kvp.second) << MW
             << std::endl;
  }
  expected << std::endl;
  expected << "--- Start sensitivity analysis with epsilon = "
           << input_data.epsilon << EUROS << " --- " << std::endl;
  expected << std::endl;

  _logger.log_at_start(input_data);
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogBeginPbResolution) {
  auto pb_data = SinglePbData(SensitivityPbType::CAPEX, str_pb_type,
                              "some_name", "my_opt_dir");
  std::stringstream expected;
  expected << indent_1 << "Solving " << pb_data.get_pb_description() << "..."
           << std::endl;

  _logger.log_begin_pb_resolution(pb_data);
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSolutionProjection) {
  auto pb_data =
      SinglePbData(SensitivityPbType::PROJECTION, str_pb_type, "some_name",
                   "my_opt_dir", 10, 1e6, {{"peak", 15}, {"semibase", 50}}, 0);

  std::stringstream expected;
  expected << indent_1 << indent_1 << pb_data.get_pb_description() << " = "
           << xpansion::logger::commons::create_str_mw(pb_data.objective) << MW
           << std::endl;
  expected << indent_1 << indent_1 << "Overall cost = "
           << xpansion::logger::commons::create_str_million_euros(
                  pb_data.system_cost)
           << MILLON_EUROS << std::endl;
  expected << std::endl;

  _logger.log_pb_solution(pb_data);
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSolutionCapex) {
  auto pb_data =
      SinglePbData(SensitivityPbType::CAPEX, str_pb_type, "some_name",
                   "my_opt_dir", 10, 1e6, {{"peak", 15}, {"semibase", 50}}, 0);

  std::stringstream expected;
  expected << indent_1 << indent_1 << pb_data.get_pb_description() << " = "
           << xpansion::logger::commons::create_str_million_euros(
                  pb_data.objective)
           << MILLON_EUROS << std::endl;
  expected << indent_1 << indent_1 << "Overall cost = "
           << xpansion::logger::commons::create_str_million_euros(
                  pb_data.system_cost)
           << MILLON_EUROS << std::endl;
  expected << std::endl;

  _logger.log_pb_solution(pb_data);
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogAtEnding) {
  std::stringstream expected;
  expected << "--- Sensitivity study completed ---" << std::endl;

  _logger.log_at_ending();
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSummaryOnlyCapex) {
  std::map<std::string, std::pair<double, double>> candidates_bounds = {
      {peak_name, {0, 100}}, {semibase_name, {0, 200}}};

  SensitivityInputData input_data = {
      epsilon, best_benders_cost, 0,    {}, {}, nullptr,
      "",      candidates_bounds, true, {}};

  auto capex_min_data =
      SinglePbData(SensitivityPbType::CAPEX, str_pb_type, "", MIN_C, 1040, 1390,
                   {{peak_name, 14}, {semibase_name, 10}}, 42);

  auto capex_max_data =
      SinglePbData(SensitivityPbType::CAPEX, str_pb_type, "", MAX_C, 1224, 1490,
                   {{peak_name, 17}, {semibase_name, 11}}, 37);

  std::vector<SinglePbData> pbs_data = {capex_min_data, capex_max_data};

  std::stringstream expected;
  expected << std::endl
           << "--- Sensitivity analysis summary "
           << "(epsilon = " << input_data.epsilon << EUROS << ") ---"
           << std::endl
           << std::endl;

  expected << indent_1 << "CAPEX interval: ["
           << xpansion::logger::commons::create_str_million_euros(1040) << ", "
           << xpansion::logger::commons::create_str_million_euros(1224) << "]"
           << MILLON_EUROS << std::endl
           << std::endl;

  _logger.log_summary(input_data, pbs_data);
  EXPECT_EQ(_stream.str(), expected.str());
}

TEST_F(SensitivityUserLoggerTest, LogPbSummaryOnlyProjection) {
  std::map<std::string, std::pair<double, double>> candidates_bounds = {
      {peak_name, {0, 100}}, {semibase_name, {0, 200}}};

  SensitivityInputData input_data = {
      epsilon, best_benders_cost, 0,    {}, {}, nullptr,
      "",      candidates_bounds, true, {}};

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

  std::stringstream expected;
  expected << std::endl
           << "--- Sensitivity analysis summary "
           << "(epsilon = " << input_data.epsilon << EUROS << ") ---"
           << std::endl
           << std::endl;

  expected << indent_1 << "Investment intervals of candidates:" << std::endl;
  expected << indent_1 << indent_1 << peak_name << ": ["
           << xpansion::logger::commons::create_str_mw(13) << ", "
           << xpansion::logger::commons::create_str_mw(24) << "]" << MW
           << indent_1 << "-- possible interval = ["
           << xpansion::logger::commons::create_str_mw(0) << ", "
           << xpansion::logger::commons::create_str_mw(100) << "]" << MW
           << std::endl;

  expected << indent_1 << indent_1 << semibase_name << ": ["
           << xpansion::logger::commons::create_str_mw(10) << ", "
           << xpansion::logger::commons::create_str_mw(11) << "]" << MW
           << indent_1 << "-- possible interval = ["
           << xpansion::logger::commons::create_str_mw(0) << ", "
           << xpansion::logger::commons::create_str_mw(200) << "]" << MW
           << std::endl;
  expected << std::endl;

  _logger.log_summary(input_data, pbs_data);
  EXPECT_EQ(_stream.str(), expected.str());
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

  void log_at_start(const SensitivityInputData& input_data) override {
    _initCall = true;
  }

  void log_begin_pb_resolution(const SinglePbData& pb_data) override {
    _beginPbResolutionCall = true;
  }

  void log_pb_solution(const SinglePbData& pb_data) override {
    _pbSolutionCall = true;
  }

  void log_summary(const SensitivityInputData& input_data,
                   const std::vector<SinglePbData>& pbs_data) override {
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
  EXPECT_EQ(_logger->_displaymessage, message);
  EXPECT_EQ(_logger2->_displaymessage, message);
}

TEST_F(SensitivityMasterLoggerTest, InitLog) {
  SensitivityInputData input_data;
  _master.log_at_start(input_data);
  EXPECT_TRUE(_logger->_initCall);
  EXPECT_TRUE(_logger2->_initCall);
}

TEST_F(SensitivityMasterLoggerTest, BeginPbResolutionLog) {
  SinglePbData pb_data;
  _master.log_begin_pb_resolution(pb_data);
  EXPECT_TRUE(_logger->_beginPbResolutionCall);
  EXPECT_TRUE(_logger2->_beginPbResolutionCall);
}

TEST_F(SensitivityMasterLoggerTest, PbSolutionLog) {
  SinglePbData pb_data;
  _master.log_pb_solution(pb_data);
  EXPECT_TRUE(_logger->_pbSolutionCall);
  EXPECT_TRUE(_logger2->_pbSolutionCall);
}

TEST_F(SensitivityMasterLoggerTest, SummaryLog) {
  SensitivityInputData input_data;
  std::vector<SinglePbData> pbs_data;
  _master.log_summary(input_data, pbs_data);
  EXPECT_TRUE(_logger->_summaryCall);
  EXPECT_TRUE(_logger2->_summaryCall);
}

TEST_F(SensitivityMasterLoggerTest, EndingLog) {
  _master.log_at_ending();
  EXPECT_TRUE(_logger->_endingCall);
  EXPECT_TRUE(_logger2->_endingCall);
}