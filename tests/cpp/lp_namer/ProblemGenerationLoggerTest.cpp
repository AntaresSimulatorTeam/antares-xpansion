
#include "LoggerBuilder.h"
#include "ProblemGenerationLogger.h"
#include "gtest/gtest.h"
using namespace ProblemGenerationLog;
class ProblemGenerationLoggerTest : public ::testing::Test {
 public:
  std::filesystem::path temp_file_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;

 protected:
};

TEST_F(ProblemGenerationLoggerTest, fileLogIsCreated) {
  temp_file_ = std::filesystem::temp_directory_path() /
               "ProblemGenerationLoggerTestFile.txt";
  logger_ = BuildLoggerWithParams(LOGLEVEL::INFO, temp_file_);
  logger_ << "HELLO;";
  ASSERT_EQ(std::filesystem::exists(temp_file_), true);
}
TEST_F(ProblemGenerationLoggerTest, MessageAreWritenInStream) {
  temp_file_ = std::filesystem::temp_directory_path() /
               "ProblemGenerationLoggerTestFile.txt";
  std::stringstream stream;

  logger_ = BuildLoggerWithParams(LOGLEVEL::INFO, stream);
  auto msg = "HELLO";
  (*logger_)() << msg;
  auto expectedMsg = logger_->PrefixMessage() + msg;
  ASSERT_EQ(expectedMsg, stream.str());
}
