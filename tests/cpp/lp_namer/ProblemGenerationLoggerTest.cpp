
#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "gtest/gtest.h"
using namespace ProblemGenerationLog;
class ProblemGenerationLoggerTest : public ::testing::Test {
 public:
  std::filesystem::path temp_file_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;

};

TEST_F(ProblemGenerationLoggerTest, fileLogIsCreated) {
  temp_file_ = std::filesystem::temp_directory_path() /
               "ProblemGenerationLoggerTestFile.txt";
  logger_ = BuildLoggerWithParams(LogUtils::LOGLEVEL::INFO, temp_file_);
  (*logger_)(LogUtils::LOGLEVEL::INFO) << "HELLO;";
  ASSERT_EQ(std::filesystem::exists(temp_file_), true);
}
TEST_F(ProblemGenerationLoggerTest, MessageAreWritenInStream) {
  temp_file_ = std::filesystem::temp_directory_path() /
               "ProblemGenerationLoggerTestFile.txt";
  std::stringstream stream;

  logger_ = BuildLoggerWithParams(LogUtils::LOGLEVEL::INFO, stream);
  std::string expectedMsg ("HELLO");
  (*logger_)() << expectedMsg;
  auto printedMsg = stream.str();
  auto printedMsgSize = printedMsg.size();
  ASSERT_EQ(expectedMsg, printedMsg.substr(printedMsgSize-expectedMsg.size()) );
}
