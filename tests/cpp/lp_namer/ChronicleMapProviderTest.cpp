//
// Created by marechaljas on 28/04/2022.
//

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <utility>

#include "antares-xpansion/lpnamer/model/ChronicleMapProvider.h"
#include "antares-xpansion/lpnamer/model/ChronicleMapReader.h"
#include "LoggerBuilder.h"

class ChronicleProviderTest : public ::testing::Test {
 public:
  std::filesystem::path ts_info_root_ = std::filesystem::temp_directory_path();
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_ =
      emptyLogger();

 protected:
  void SetUp() override {
    std::filesystem::create_directories(ts_info_root_ / "A");
    std::ofstream b_file(ts_info_root_ / "A" / "B.txt");
    b_file << "Garbage\n42\n52\n";
    b_file.close();
    std::filesystem::create_directories(ts_info_root_ / "B");
    std::ofstream c_file(ts_info_root_ / "B" / "C.txt");
    c_file << "Garbage\n99\n2";
    c_file.close();
  }
};

TEST_F(ChronicleProviderTest, GetMapForLinkAToB) {
  DirectAccessScenarioToChronicleProvider provider(ts_info_root_, logger_);
  std::map<unsigned, unsigned> res = provider.GetMap("A", "B");
  std::map<unsigned, unsigned> expected = {{1, 42}, {2, 52}};
  ASSERT_EQ(res, expected);
}

TEST_F(ChronicleProviderTest, GetMapForUnorderedLinks) {
  DirectAccessScenarioToChronicleProvider provider(ts_info_root_, logger_);
  std::map<unsigned, unsigned> res = provider.GetMap("C", "B");
  std::map<unsigned, unsigned> expected = {{1, 99}, {2, 2}};
  ASSERT_EQ(res, expected);
}

TEST_F(ChronicleProviderTest, EmptyMapIfNoInfoOnDisk) {
  DirectAccessScenarioToChronicleProvider provider(ts_info_root_, logger_);
  std::map<unsigned, unsigned> res = provider.GetMap("Somwhere", "Elsewhere");
  ASSERT_TRUE(res.empty());
}

// From Directory To File
// Order link aplphabetique