#include <gtest/gtest.h>

#include "LoggerBuilder.h"
#include "LpFilesExtractor.h"

const auto LP_FILE_EXTRACTOR_TEST_DIR =
    std::filesystem::path("data_test") / "tests_lpnamer" / "LpFilesExtractor";
const std::filesystem::path EMPTY_ARCHIVE =
    LP_FILE_EXTRACTOR_TEST_DIR / "empty_archive.zip";
const std::filesystem::path TWO_AREAS_FILES_ARCHIVE =
    LP_FILE_EXTRACTOR_TEST_DIR / "2_areas_files.zip";
const std::filesystem::path THREE_INTERCOS_FILES_ARCHIVE =
    LP_FILE_EXTRACTOR_TEST_DIR / "3_intercos_files.zip";
const std::filesystem::path ONE_AREA_0_INTERCO =
    LP_FILE_EXTRACTOR_TEST_DIR / "test_0_interco_1_area.zip";

class LpFilesExtractorTest : public ::testing::Test {
 protected:
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_ =
      emptyLogger();
};

TEST_F(LpFilesExtractorTest, IfNoAreaFileIsInAntaresArchive) {
  auto lp_files_extractor = LpFilesExtractor(EMPTY_ARCHIVE, std::filesystem::temp_directory_path(),
                       logger_, SimulationInputMode::ARCHIVE);

  std::ostringstream expectedErrorString;
  expectedErrorString << "No area*.txt file found" << std::endl;

  try {
    lp_files_extractor.ExtractFiles();
  } catch (const LpFilesExtractor::ErrorWithAreaFile& e) {
    EXPECT_EQ(e.ErrorMessage(), expectedErrorString.str());
  }
}
TEST_F(LpFilesExtractorTest, IfMoreThanOneAreaFileFoundInAntaresArchive) {
  auto lp_files_extractor = LpFilesExtractor(
      TWO_AREAS_FILES_ARCHIVE, std::filesystem::temp_directory_path(), logger_,
      SimulationInputMode::ARCHIVE);

  std::ostringstream expectedErrorString;
  expectedErrorString << "More than one area*.txt file found" << std::endl;

  try {
    lp_files_extractor.ExtractFiles();
  } catch (const LpFilesExtractor::ErrorWithAreaFile& e) {
    EXPECT_EQ(e.ErrorMessage(), expectedErrorString.str());
  }
}
TEST_F(LpFilesExtractorTest, IfNoIntercoFileIsInAntaresArchive) {
  auto out_dir = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  std::filesystem::create_directory(out_dir);
  auto lp_files_extractor = LpFilesExtractor(ONE_AREA_0_INTERCO, out_dir, logger_, SimulationInputMode::ARCHIVE);

  std::ostringstream expectedErrorString;
  expectedErrorString << "No interco*.txt file found" << std::endl;

  try {
    lp_files_extractor.ExtractFiles();
  } catch (const LpFilesExtractor::ErrorWithIntercosFile& e) {
    EXPECT_EQ(e.ErrorMessage(), expectedErrorString.str());
  }
}
TEST_F(LpFilesExtractorTest, IfMoreThanOneIntercoFileFoundInAntaresArchive) {
  auto out_dir = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  std::filesystem::create_directory(out_dir);
  auto lp_files_extractor = LpFilesExtractor(THREE_INTERCOS_FILES_ARCHIVE,
                                             out_dir, logger_,
                       SimulationInputMode::ARCHIVE);

  std::ostringstream expectedErrorString;
  expectedErrorString << "More than one interco*.txt file found" << std::endl;

  try {
    lp_files_extractor.ExtractFiles();
  } catch (const LpFilesExtractor::ErrorWithIntercosFile& e) {
    EXPECT_EQ(e.ErrorMessage(), expectedErrorString.str());
  }
}
