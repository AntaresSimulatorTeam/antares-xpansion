#include "WeightsFileReader.h"
#include "gtest/gtest.h"

class WeightsFileReaderTest : public ::testing::Test {
 protected:
  void SetUp() {}
};

TEST_F(WeightsFileReaderTest, FailsIfWeightsFileDoesNotExist) {
  auto weights_file_reader = WeightsFileReader("", {});
  ASSERT_FALSE(weights_file_reader.CheckWeightsFile());
}