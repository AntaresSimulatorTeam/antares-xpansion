#include <fstream>
#include <vector>

#include "antares-xpansion/helpers/ArchiveWriter.h"
#include "MpsTxtWriter.h"
#include "gtest/gtest.h"

const std::vector<MpsVariableConstraintsFiles> EXPECTED_RESULTS = {
    {"problem-1-1.mps", "variables-1-1.txt", "constraints-1-1.txt"},
    {"problem-1-2.mps", "variables-1-2.txt", "constraints-1-2.txt"},
    {"problem-1-3.mps", "variables-1-3.txt", "constraints-1-3.txt"},
    {"problem-2-1.mps", "variables-2-1.txt", "constraints-2-1.txt"},
    {"problem-2-2.mps", "variables-2-2.txt", "constraints-2-2.txt"},
    {"problem-2-3.mps", "variables-2-3.txt", "constraints-2-3.txt"}};

auto const empty_files_archive = std::filesystem::path("data_test") /
                                 "tests_lpnamer" / "MpsTxtWriterTest" /
                                 "MpsTxtWriterTest_CheckMpsTxtContent.zip";
class MpsTxtWriterTest : public ::testing::Test {};

TEST_F(MpsTxtWriterTest, CheckMpsTxtContent) {
  auto files_mapper = FilesMapper(empty_files_archive);
  auto year_week_files = files_mapper.FilesMap();
  std::vector<MpsVariableConstraintsFiles> files_map;

  std::transform(year_week_files.cbegin(), year_week_files.cend(),
                 std::back_inserter(files_map),
                 [](const std::pair<YearAndWeek, MpsVariableConstraintsFiles>&
                        year_week_files) {
                   auto& [year_week, files] = year_week_files;
                   return files;
                 });

  ASSERT_EQ(EXPECTED_RESULTS, files_map);
}
