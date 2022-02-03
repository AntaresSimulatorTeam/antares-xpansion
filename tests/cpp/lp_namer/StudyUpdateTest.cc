#include <fstream>

#include "ActiveLinks.h"
#include "CandidatesINIReader.h"
#include "LinkProfileReader.h"
#include "StudyUpdater.h"
#include "common_lpnamer.h"
#include "gtest/gtest.h"
#include "helpers/Path.h"
class StudyUpdateTest : public ::testing::Test {
 protected:
  static std::vector<ActiveLink> _links;

  static void SetUpTestCase() {
    // Change capacity folder to retrieve temp_profile.ini in the current
    // location

    // called before 1st test
    std::string content_l;
    std::ofstream file_l;

    // dummy interco tmp file name
    file_l.open("temp_interco.txt");
    content_l =
        "\
0 0 1\n\
1 0 2\n\
";
    file_l << content_l;
    file_l.close();

    // dummy area tmp file name
    file_l.open("temp_area.txt");
    content_l =
        "\
area1\n\
area2\n\
peak\n\
";
    file_l << content_l;
    file_l.close();

    // dummy candidates tmp file name
    file_l.open("temp_candidates.ini");
    content_l =
        "\
[1]\n\
name = peak\n\
link = area1 - peak\n\
annual-cost-per-mw = 60000\n\
unit-size = 100\n\
max-units = 20\n\
link-profile = temp_profile.ini\n\
\n\
\n\
[2]\n\
name = transmission_line\n\
link = area1 - area2\n\
annual-cost-per-mw = 10000\n\
unit-size = 400\n\
max-units = 8\n\
already-installed-capacity = 100\n\
";
    file_l << content_l;
    file_l.close();

    // dummy link profile tmp file name
    file_l.open("temp_profile.ini");
    std::vector<double> directLinkprofile_l(8760, 1);
    directLinkprofile_l[0] = 0;
    directLinkprofile_l[1] = 0.5;
    std::vector<double> indirectLinkprofile_l(8760, 1);
    indirectLinkprofile_l[0] = 0.25;
    indirectLinkprofile_l[1] = 0.75;

    for (auto cnt_l = 0; cnt_l < directLinkprofile_l.size() - 1; ++cnt_l) {
      file_l << directLinkprofile_l[cnt_l] << "\t"
             << indirectLinkprofile_l[cnt_l] << "\n";
    }
    file_l << directLinkprofile_l.back() << "\t"
           << indirectLinkprofile_l.back();
    file_l.close();

    CandidatesINIReader candidateReader("temp_interco.txt", "temp_area.txt");
    LinkProfileReader profileReader;

    std::vector<CandidateData> cand_data_list =
        candidateReader.readCandidateData("temp_candidates.ini");

    std::map<std::string, LinkProfile> profile_map =
        profileReader.getLinkProfileMap(".", cand_data_list);

    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map};
    StudyUpdateTest::_links = linkBuilder.getLinks();
  }

  static void TearDownTestCase() {
    // called after last test

    // delete the created tmp file
    std::remove("temp_interco.txt");
    std::remove("temp_area.txt");
    std::remove("temp_candidates.ini");
    std::remove("temp_profile.ini");
  }

  void SetUp() {
    // called before each test
  }

  void TearDown() {
    // called after each test
  }
};

std::vector<ActiveLink> StudyUpdateTest::_links;

/***
 Verify :
    * two link created
    * two candidates created (one on each link):
    *   - peak
    *   - transmission_line
***/
TEST_F(StudyUpdateTest, linksInit) {
  ASSERT_EQ(_links.size(), 2);

  const auto &candidatesLink0 = _links[0].getCandidates();
  const auto &candidatesLink1 = _links[1].getCandidates();
  ASSERT_EQ(candidatesLink0.size(), 1);
  ASSERT_EQ(candidatesLink1.size(), 1);

  ASSERT_EQ(candidatesLink0[0].get_name(), "transmission_line");
  ASSERT_EQ(candidatesLink1[0].get_name(), "peak");
}

/***
 Verify :
    * first candidate has a link profile read from file :
        - verify first 3 elements
        - verify last one (ie 8759)
        - invalid indices return 0
***/
TEST_F(StudyUpdateTest, linkprofile) {
  ASSERT_EQ(_links.size(), 2);
  const auto &link1candidates = _links[1].getCandidates();

  ASSERT_EQ(link1candidates.size(), 1);

  // direct profile
  ASSERT_EQ(link1candidates[0].direct_profile(0), 0);
  ASSERT_EQ(link1candidates[0].direct_profile(1), 0.5);
  ASSERT_EQ(link1candidates[0].direct_profile(2), 1);
  ASSERT_EQ(link1candidates[0].direct_profile(8759), 1);

  // indirect profile
  ASSERT_EQ(link1candidates[0].indirect_profile(0), 0.25);
  ASSERT_EQ(link1candidates[0].indirect_profile(1), 0.75);
  ASSERT_EQ(link1candidates[0].indirect_profile(2), 1);
  ASSERT_EQ(link1candidates[0].indirect_profile(8759), 1);
}

TEST_F(StudyUpdateTest, defaultAntaresVersion) {
  StudyUpdater studyupdater(".");
  ASSERT_EQ(studyupdater.getAntaresVersion(), 710);
}

TEST_F(StudyUpdateTest, antaresVersion700) {
  std::string content_l;
  std::ofstream file_l("study.antares");

  // dummy study tmp file name
  content_l =
      "\
[antares]\n\
version = 700\n\
caption = test_case_7.1_structure\n\
created = 1455269769\n\
lastsave = 1592911186\n\
author = Unknown\n\
";
  file_l << content_l;
  file_l.close();

  StudyUpdater studyupdater(".");
  ASSERT_EQ(studyupdater.getAntaresVersion(), 700);

  std::remove("study.antares");
}

/***
 Verify :
    * returned locations of the link data files
    * (ie for each link ORIGIN - DESTINATION, they are in
studypath/input/links/ORIGIN/DESTINATION.txt)
***/
TEST_F(StudyUpdateTest, LinkFilenames) {
  StudyUpdater studyupdater(".");
  ASSERT_EQ(studyupdater.getLinkdataFilepath(_links[0]),
            static_cast<std::string>(Path(".") / "input" / "links" / "area1" /
                                     "area2.txt"));
  ASSERT_EQ(studyupdater.getLinkdataFilepath(_links[1]),
            static_cast<std::string>(Path(".") / "input" / "links" / "area1" /
                                     "peak.txt"));
}

TEST_F(StudyUpdateTest, computeNewCapacities) {
  StudyUpdater studyupdater(".");

  // candidate peak has a link profile
  const std::map<std::string, double> &investissments = {
      {"peak", 1000}, {"transmission_line", 0}};

  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[1], 0),
            std::make_pair(0., 250.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[1], 1),
            std::make_pair(500., 750.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[1], 2),
            std::make_pair(1000., 1000.));

  // link 0 : area1 - area2 has an already installed capacity of 100
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[0], 0),
            std::make_pair(100., 100.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[0], 1),
            std::make_pair(100., 100.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[0], 2),
            std::make_pair(100., 100.));
}

TEST_F(StudyUpdateTest, no_computed_investment_for_candidate_peak) {
  StudyUpdater studyupdater(".");

  // candidate peak has no computed investments
  const std::map<std::string, double> &investissments = {
      {"transmission_line", 0}};

  try {
    studyupdater.computeNewCapacities(investissments, _links[1], 0);
    FAIL() << "Missing investment not detected for candidate peak on link "
              "area1 - peak";
  } catch (const std::runtime_error &err) {
    ASSERT_STREQ(err.what(),
                 "No investment computed for the candidate peak on "
                 "the link area1 - peak");
  }
}
