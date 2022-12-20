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
  const auto invalid_path = "";
  auto weights_file_reader = WeightsFileReader(invalid_path, {}, logger_);
  std::ostringstream expectedErrorString;
  expectedErrorString << "Could not open weights file: " << invalid_path
                      << std::endl;

  try {
    weights_file_reader.CheckWeightsFile();
  } catch (const WeightsFileReader::WeightsFileOpenError& e) {
    EXPECT_EQ(e.what(), expectedErrorString.str());
  }
}