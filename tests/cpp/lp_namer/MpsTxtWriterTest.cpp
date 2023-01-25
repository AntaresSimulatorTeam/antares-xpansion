#include <fstream>
#include <vector>

#include "ArchiveWriter.h"
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
  auto mps_txt = std::filesystem::temp_directory_path() / "mps.txt";
  auto mps_txt_writer = MpsTxtWriter(empty_files_archive, mps_txt);
  mps_txt_writer.Write();

  ASSERT_TRUE(std::filesystem::exists(mps_txt));

  std::ifstream mps_txt_file(mps_txt);
  std::string line;
  std::vector<MpsVariableConstraintsFiles> mps_txt_content;
  while (std::getline(mps_txt_file, line)) {
    std::istringstream instream(line);
    std::string mps_file;
    std::string vars_file;
    std::string constraints_file;
    instream >> mps_file >> vars_file >> constraints_file;
    ;
    mps_txt_content.push_back({mps_file, vars_file, constraints_file});
  }

  ASSERT_EQ(EXPECTED_RESULTS, mps_txt_content);
}
