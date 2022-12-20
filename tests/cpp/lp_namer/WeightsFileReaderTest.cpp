#include "WeightsFileReader.h"
#include "gtest/gtest.h"
using namespace ProblemGenerationLog;
class WeightsFileReaderTest : public ::testing::Test {
 protected:
  ProblemGenerationLoggerSharedPointer logger_;

  void SetUp() {
    logger_ = std::make_shared<ProblemGenerationLogger>(LOGLEVEL::NONE);
  }
};

TEST_F(WeightsFileReaderTest, FailsIfWeightsFileDoesNotExist) {
  auto weights_file_reader = WeightsFileReader("", {}, logger_);
  ASSERT_FALSE(weights_file_reader.CheckWeightsFile());
}