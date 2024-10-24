#include <fstream>

#include "LoggerBuilder.h"
#include "antares-xpansion/helpers/AreaParser.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/CandidatesINIReader.h"
#include "gtest/gtest.h"

const std::string interco_content_l =
    "0 0 1\n"
    "1 0 3\n"
    "2 0 5\n"
    "3 1 2\n"
    "4 1 4";

const std::string area_content_l =
    "area1\n"
    "area2\n"
    "flex\n"
    "peak\n"
    "pv\n"
    "semibase";

const std::string candidate_content_l =
    "[1]\n"
    "name = semibase\n"
    "link = area1 - semibase\n"
    "annual-cost-per-mw = 126000\n"
    "unit-size = 200\n"
    "max-units = 10\n"
    "enable = true\n"
    "already-installed-direct-link-profile = "
    "alreadyInstalledDirectProfile.txt\n"
    "already-installed-indirect-link-profile = "
    "alreadyInstalledIndirectProfile.txt\n"
    "direct-link-profile = directLinkProfile.txt\n"
    "indirect-link-profile = indirectLinkProfile.txt\n"
    "\n"
    "[2]\n"
    "name = peak\n"
    "link = area1 - peak\n"
    "annual-cost-per-mw = 60000\n"
    "unit-size = 100\n"
    "max-units = 20\n"
    "enable = false\n"
    "\n"
    "[3]\n"
    "name = peak2\n"
    "link = area1 - peak\n"
    "annual-cost-per-mw = 30000\n"
    "unit-size = 100\n"
    "max-units = 20";

class CandidatesINIReaderTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    // called before 1st test

    // dummy interco tmp file name
    std::ofstream file_interco("temp_interco.txt");
    file_interco << interco_content_l;
    file_interco.close();

    // dummy area tmp file name
    std::ofstream file_area("temp_area.txt");
    file_area << area_content_l;
    file_area.close();

    // dummy area tmp file name
    std::ofstream file_candidate("temp_candidate.ini");
    file_candidate << candidate_content_l;
    file_candidate.close();
  }

  static void TearDownTestCase() {
    // called after last test

    // delete the created tmp file
    std::remove("temp_interco.txt");
    std::remove("temp_area.txt");
    std::remove("temp_candidate.ini");
  }

  void SetUp() {
    // called before each test
  }

  void TearDown() {
    // called after each test
  }

 public:
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer emptyLogger_ =
      emptyLogger();
};

TEST_F(CandidatesINIReaderTest, testReadIntero) {
  std::vector<IntercoFileData> intercoDataList =
      CandidatesINIReader(emptyLogger_)
          .ReadAntaresIntercoFile("temp_interco.txt");

  ASSERT_EQ(intercoDataList[0].index_interco, 0);
  ASSERT_EQ(intercoDataList[0].index_pays_origine, 0);
  ASSERT_EQ(intercoDataList[0].index_pays_extremite, 1);
}

TEST_F(CandidatesINIReaderTest, testReadArea) {
  const auto areaFileData = AreaParser::ReadAreaFile("temp_area.txt");

  const auto& areaList = areaFileData.areas;
  ASSERT_EQ(areaList[0], "area1");
  ASSERT_EQ(areaList[1], "area2");
  ASSERT_EQ(areaList[2], "flex");
}

TEST_F(CandidatesINIReaderTest, testReadCandidate) {
  CandidatesINIReader reader("temp_interco.txt", "temp_area.txt", emptyLogger_);

  std::vector<CandidateData> candidates_data =
      reader.readCandidateData("temp_candidate.ini");

  ASSERT_EQ(candidates_data.size(), 3);

  ASSERT_EQ(candidates_data[0].name, "semibase");
  ASSERT_EQ(candidates_data[0].link_name, "area1 - semibase");
  ASSERT_EQ(candidates_data[0].link_id, 2);
  ASSERT_EQ(candidates_data[0].enable, true);
  ASSERT_DOUBLE_EQ(candidates_data[0].annual_cost_per_mw, 126000);

  ASSERT_EQ(candidates_data[1].name, "peak");
  ASSERT_EQ(candidates_data[1].link_name, "area1 - peak");
  ASSERT_EQ(candidates_data[1].link_id, 1);
  ASSERT_EQ(candidates_data[1].enable, false);
  ASSERT_DOUBLE_EQ(candidates_data[1].annual_cost_per_mw, 60000);

  ASSERT_EQ(candidates_data[2].name, "peak2");
  ASSERT_EQ(candidates_data[2].link_name, "area1 - peak");
  ASSERT_EQ(candidates_data[2].link_id, 1);
  ASSERT_EQ(candidates_data[2].enable, true);
  ASSERT_DOUBLE_EQ(candidates_data[2].annual_cost_per_mw, 30000);
}

TEST_F(CandidatesINIReaderTest, AcceptLinkProfileKey) {
  CandidatesINIReader reader("temp_interco.txt", "temp_area.txt", emptyLogger_);

  std::vector<CandidateData> candidates_data =
      reader.readCandidateData("temp_candidate.ini");

  ASSERT_EQ(candidates_data.at(0).direct_link_profile, "directLinkProfile.txt");
  ASSERT_EQ(candidates_data.at(0).indirect_link_profile,
            "indirectLinkProfile.txt");
}

TEST_F(CandidatesINIReaderTest, AcceptAlreadyInstalledLinkProfileKey) {
  CandidatesINIReader reader("temp_interco.txt", "temp_area.txt", emptyLogger_);

  std::vector<CandidateData> candidates_data =
      reader.readCandidateData("temp_candidate.ini");

  ASSERT_EQ(candidates_data.at(0).installed_direct_link_profile_name,
            "alreadyInstalledDirectProfile.txt");
  ASSERT_EQ(candidates_data.at(0).installed_indirect_link_profile_name,
            "alreadyInstalledIndirectProfile.txt");
}