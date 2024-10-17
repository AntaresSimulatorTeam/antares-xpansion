#include <cstdio>

#include "antares-xpansion/lpnamer/input_reader/WeightsFileReader.h"
#include "gtest/gtest.h"
using namespace ProblemGenerationLog;
class WeightsFileReaderTest : public ::testing::Test {
 protected:
  ProblemGenerationLoggerSharedPointer logger_;

  void SetUp() {
    logger_ = std::make_shared<ProblemGenerationLogger>(LogUtils::LOGLEVEL::NONE);
  }
};

TEST_F(WeightsFileReaderTest, FailsIfWeightsFileDoesNotExist) {
  const auto invalid_path = "";
  auto weights_file_reader = WeightsFileReader(invalid_path, 0, logger_);
  std::ostringstream expectedErrorString;
  expectedErrorString << "Could not open weights file: " << invalid_path
                      << std::endl;

  try {
    weights_file_reader.CheckWeightsFile();
  } catch (const WeightsFileReader::WeightsFileOpenError& e) {
    EXPECT_EQ(e.ErrorMessage().c_str(), expectedErrorString.str());
  }
}

TEST_F(WeightsFileReaderTest, FailsIfEmptyWeightFileIsGiven) {
  auto tmp_path =
      std::filesystem::temp_directory_path() / "Empty-WeightsFile.txt";
  std::ofstream writer(tmp_path);
  writer.close();
  auto weights_file_reader = WeightsFileReader(tmp_path, 0, logger_);
  std::ostringstream expectedErrorString;
  expectedErrorString << "Error! Weights file is empty: " << tmp_path.string()
                      << std::endl;

  try {
    weights_file_reader.CheckWeightsFile();
  } catch (const WeightsFileReader::WeightsFileIsEmtpy& e) {
    EXPECT_EQ(e.ErrorMessage().c_str(), expectedErrorString.str());
  }
}

TEST_F(WeightsFileReaderTest, FailsIfAllWeightsValueAreNull) {
  auto tmp_path =
      std::filesystem::temp_directory_path() / "Empty-WeightsFile.txt";
  std::ofstream writer(tmp_path);
  writer << 0 << std::endl << 0 << std::endl;
  writer.close();
  auto weights_file_reader = WeightsFileReader(tmp_path, 2, logger_);
  std::ostringstream expectedErrorString;
  expectedErrorString << "file " << tmp_path.string() << ": all values are null"
                      << std::endl;

  try {
    weights_file_reader.CheckWeightsFile();
  } catch (const WeightsFileReader::OnlyNullYearsWeightValue& e) {
    EXPECT_EQ(e.ErrorMessage().c_str(), expectedErrorString.str());
  }
}

TEST_F(WeightsFileReaderTest, FailsIfStringLineIsWritenInWeightsFile) {
  auto tmp_path = std::filesystem::temp_directory_path() /
                  "FailsIfStringLineIsWritenInWeightsFile.txt";
  std::ofstream writer(tmp_path);
  writer << "frfr\n"
         << "35\n";
  writer.close();
  auto weights_file_reader = WeightsFileReader(tmp_path, {}, logger_);
  std::ostringstream expectedErrorString;
  expectedErrorString << "Line " << 0 << " in file " << tmp_path.string()
                      << "is not a single non-negative value" << std::endl;

  try {
    weights_file_reader.CheckWeightsFile();
  } catch (const WeightsFileReader::InvalidYearsWeightValue& e) {
    EXPECT_EQ(e.ErrorMessage().c_str(), expectedErrorString.str());
  }
}

TEST_F(WeightsFileReaderTest, FailsIfNegativeValueIsGivenInWeightsFile) {
  auto tmp_path = std::filesystem::temp_directory_path() /
                  "FailsIfNegativeValueIsGivenInWeightsFile.txt";
  std::ofstream writer(tmp_path);
  writer << 2 << std::endl << -3 << std::endl;
  writer.close();
  auto weights_file_reader = WeightsFileReader(tmp_path, {}, logger_);
  std::ostringstream expectedErrorString;
  expectedErrorString << "Line " << 2 << " in file " << tmp_path.string()
                      << " indicates a negative value" << std::endl;

  try {
    weights_file_reader.CheckWeightsFile();
  } catch (const WeightsFileReader::InvalidYearsWeightValue& e) {
    EXPECT_EQ(e.ErrorMessage().c_str(), expectedErrorString.str());
  }
}