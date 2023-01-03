#include <gtest/gtest.h>

#include "LoggerBuilder.h"
#include "LpFilesExtractor.h"

const std::filesystem::path empty_archive =
    std::filesystem::path("data_test") / "tests_lpnamer" / "LpFilesExtractor" /
    "empty_archive.zip";
class LpFilesExtractorTest : public ::testing::Test {
 protected:
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_ =
      emptyLogger();
};

TEST_F(LpFilesExtractorTest, IfNoAreaFileIsInAntaresArchive) {
  auto lp_files_extractor = LpFilesExtractor(
      empty_archive, std::filesystem::temp_directory_path(), logger_);

  std::ostringstream expectedErrorString;
  expectedErrorString << "No area*.txt file found" << std::endl;

  try {
    lp_files_extractor.ExtractFiles();
  } catch (const LpFilesExtractor::ErrorWithAreaFile& e) {
    EXPECT_EQ(e.what(), expectedErrorString.str());
  }
}
TEST_F(LpFilesExtractorTest, IfNoAreaFileIsInAntaresArchive) {
  auto lp_files_extractor = LpFilesExtractor(
      empty_archive, std::filesystem::temp_directory_path(), logger_);

  std::ostringstream expectedErrorString;
  expectedErrorString << "No area*.txt file found" << std::endl;

  try {
    lp_files_extractor.ExtractFiles();
  } catch (const LpFilesExtractor::ErrorWithAreaFile& e) {
    EXPECT_EQ(e.what(), expectedErrorString.str());
  }
}
