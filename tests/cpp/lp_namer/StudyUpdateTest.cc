#include <filesystem>
#include <fstream>

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/lpnamer/input_reader/CandidatesINIReader.h"
#include "antares-xpansion/lpnamer/input_reader/LinkProfileReader.h"
#include "LinkdataRecord.h"
#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "StudyUpdateLinkParameterStrategy.h"
#include "StudyUpdater.h"
#include "gtest/gtest.h"

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
direct-link-profile = temp_profile-direct.ini\n\
indirect-link-profile = temp_profile-indirect.ini\n\
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
    std::ofstream file_direct;
    file_direct.open("temp_profile-direct.ini");
    std::ofstream file_indirect;
    file_indirect.open("temp_profile-indirect.ini");
    std::vector<double> directLinkprofile_l(8760, 1);
    directLinkprofile_l[0] = 0;
    directLinkprofile_l[1] = 0.5;
    std::vector<double> indirectLinkprofile_l(8760, 1);
    indirectLinkprofile_l[0] = 0.25;
    indirectLinkprofile_l[1] = 0.75;

    for (auto cnt_l = 0; cnt_l < directLinkprofile_l.size() - 1; ++cnt_l) {
      file_direct << directLinkprofile_l[cnt_l] << "\n";
    }
    for (auto cnt_l = 0; cnt_l < indirectLinkprofile_l.size() - 1; ++cnt_l) {
      file_indirect << indirectLinkprofile_l[cnt_l] << "\n";
    }
    file_direct << directLinkprofile_l.back();
    file_indirect << indirectLinkprofile_l.back();
    file_direct.close();
    file_indirect.close();

    auto logger_ = emptyLogger();
    CandidatesINIReader candidateReader("temp_interco.txt", "temp_area.txt",
                                        logger_);
    LinkProfileReader profileReader(logger_);

    std::vector<CandidateData> cand_data_list =
        candidateReader.readCandidateData("temp_candidates.ini");

    std::map<std::string, std::vector<LinkProfile>> profile_map =
        profileReader.getLinkProfileMap(".", cand_data_list);

    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger_};
    StudyUpdateTest::_links = linkBuilder.getLinks();
  }

  static void TearDownTestCase() {
    // called after last test

    // delete the created tmp file
    std::remove("temp_interco.txt");
    std::remove("temp_area.txt");
    std::remove("temp_candidates.ini");
    std::remove("temp_profile-direct.ini");
    std::remove("temp_profile-indirect.ini");
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

  const auto& candidatesLink0 = _links[0].getCandidates();
  const auto& candidatesLink1 = _links[1].getCandidates();
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
  const auto& link1candidates = _links[1].getCandidates();

  ASSERT_EQ(link1candidates.size(), 1);

  // direct profile
  ASSERT_EQ(link1candidates[0].directCapacityFactor(0), 0);
  ASSERT_EQ(link1candidates[0].directCapacityFactor(1), 0.5);
  ASSERT_EQ(link1candidates[0].directCapacityFactor(2), 1);
  ASSERT_EQ(link1candidates[0].directCapacityFactor(8759), 1);

  // indirect profile
  ASSERT_EQ(link1candidates[0].indirectCapacityFactor(0), 0.25);
  ASSERT_EQ(link1candidates[0].indirectCapacityFactor(1), 0.75);
  ASSERT_EQ(link1candidates[0].indirectCapacityFactor(2), 1);
  ASSERT_EQ(link1candidates[0].indirectCapacityFactor(8759), 1);
}

/***
 Verify :
    * returned locations of the link data files
    * (ie for each link ORIGIN - DESTINATION, they are in
studypath/input/links/ORIGIN/DESTINATION.txt)
***/
TEST_F(StudyUpdateTest, LinkFilenames) {
  auto logger = emptyLogger();
  StudyUpdateLinkParameterStrategy studyupdater(std::filesystem::path("."),
                                                logger);
  ASSERT_EQ(
      studyupdater.getLinkdataFilepath(_links[0]),
      std::filesystem::path(".") / "input" / "links" / "area1" / "area2.txt");
  ASSERT_EQ(
      studyupdater.getLinkdataFilepath(_links[1]),
      std::filesystem::path(".") / "input" / "links" / "area1" / "peak.txt");
}

TEST_F(StudyUpdateTest, computeNewCapacities) {
  auto logger = emptyLogger();
  StudyUpdateLinkParameterStrategy studyupdater(".", logger);

  // candidate peak has a link profile
  const std::map<std::string, double>& investissments = {
      {"peak", 1000}, {"transmission_line", 0}};

  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[1], 0),
            std::pair(0., 250.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[1], 1),
            std::pair(500., 750.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[1], 2),
            std::pair(1000., 1000.));

  // link 0 : area1 - area2 has an already installed capacity of 100
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[0], 0),
            std::pair(100., 100.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[0], 1),
            std::pair(100., 100.));
  ASSERT_EQ(studyupdater.computeNewCapacities(investissments, _links[0], 2),
            std::pair(100., 100.));
}

TEST_F(StudyUpdateTest, no_computed_investment_for_candidate_peak) {
  auto logger = emptyLogger();
  StudyUpdateLinkParameterStrategy studyupdater(".", logger);

  // candidate peak has no computed investments
  const std::map<std::string, double>& investissments = {
      {"transmission_line", 0}};

  try {
    (void)studyupdater.computeNewCapacities(investissments, _links[1], 0);
    FAIL() << "Missing investment not detected for candidate peak on link "
              "area1 - peak";
  } catch (
      const StudyUpdateStrategy::NoInvestmentComputedForTheCandidate& err) {
    ASSERT_STREQ(err.ErrorMessage().c_str(),
                 "No investment computed for the candidate peak on the link "
                 "area1 - peak");
  }
}

namespace fs = std::filesystem;

class AntaresLinkDataReader {
 public:
  [[nodiscard]] std::vector<LinkdataRecord> Read(fs::path const& file) const {
    std::ifstream link_file(file);
    std::vector<LinkdataRecord> result;
    for (std::string line; std::getline(link_file, line); /**/) {
      LinkdataRecord record;
      record.fillFromRow(line);
      result.emplace_back(record);
    }
    return result;
  }
  [[nodiscard]] std::vector<LinkdataRecord> Read820(
      std::filesystem::path parameters_path) {
    std::ifstream link_file(parameters_path);
    std::vector<LinkdataRecord> result;
    for (std::string line; std::getline(link_file, line); /**/) {
      LinkdataRecord record;
      record.fillFromRow("42 45 " + line);
      result.emplace_back(record);
    }
    return result;
  }
};

void TestLinkDataRecord(LinkdataRecord actual, LinkdataRecord expected) {
  EXPECT_EQ(actual.fileColumns.directCapacity_,
            expected.fileColumns.directCapacity_);
  EXPECT_EQ(actual.fileColumns.indirectCapacity_,
            expected.fileColumns.indirectCapacity_);
  EXPECT_EQ(actual.fileColumns.directHurdlesCost_,
            expected.fileColumns.directHurdlesCost_);
  EXPECT_EQ(actual.fileColumns.indirectHurdlesCost_,
            expected.fileColumns.indirectHurdlesCost_);
  EXPECT_EQ(actual.fileColumns.impedances_, expected.fileColumns.impedances_);
  EXPECT_EQ(actual.fileColumns.loopFlow_, expected.fileColumns.loopFlow_);
  EXPECT_EQ(actual.fileColumns.pShiftMin_, expected.fileColumns.pShiftMin_);
  EXPECT_EQ(actual.fileColumns.pShiftMax_, expected.fileColumns.pShiftMax_);
}

void WriteRecord(std::ofstream& link_file, const LinkdataRecord& record) {
  link_file << record.fileColumns.directCapacity_ << " "
            << record.fileColumns.indirectCapacity_ << " "
            << record.fileColumns.directHurdlesCost_ << " "
            << record.fileColumns.indirectHurdlesCost_ << " "
            << record.fileColumns.impedances_ << " "
            << record.fileColumns.loopFlow_ << " "
            << record.fileColumns.pShiftMin_ << " "
            << record.fileColumns.pShiftMax_ << "\n";
}

TEST(AntaresLinkDataReaderTest, ReadLine) {
  LinkdataRecord expected_record({42, 56, 76, 400, 500, 10, 4, 6});

  AntaresLinkDataReader reader;
  fs::path link_file_path = std::tmpnam(nullptr);
  std::ofstream link_file;
  link_file.open(link_file_path);
  WriteRecord(link_file, expected_record);
  link_file.close();
  LinkdataRecord read_record = reader.Read(link_file_path).at(0);

  TestLinkDataRecord(read_record, expected_record);
}

TEST(AntaresLinkDataReaderTest, ReadFile) {
  LinkdataRecord expected_record;
  expected_record.fileColumns = {42, 56, 76, 400, 500, 1, 2, 3};

  AntaresLinkDataReader reader;
  fs::path link_file_path = std::tmpnam(nullptr);
  std::ofstream link_file;
  link_file.open(link_file_path);
  int exceptional_line_number = 56;
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    if (i == exceptional_line_number) {
      link_file << 100 << " " << 200 << " " << 300 << " " << 400 << " " << 500
                << " " << 4 << " " << 5 << " " << 6 << "\n";
    } else {
      WriteRecord(link_file, expected_record);
    }
  }
  link_file.close();
  std::vector<LinkdataRecord> read_record = reader.Read(link_file_path);

  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    if (i == exceptional_line_number) {
      LinkdataRecord expected_record_2;
      expected_record_2.fileColumns = {100, 200, 300, 400, 500, 4, 5, 6};
      TestLinkDataRecord(read_record.at(i), expected_record_2);
    } else {
      TestLinkDataRecord(read_record.at(i), expected_record);
    }
  }
}

class AntaresVersionProviderStub : public AntaresVersionProvider {
 public:
  AntaresVersionProviderStub(int version)
      : AntaresVersionProvider(), version_(version) {}

  [[nodiscard]] int getAntaresVersion(
      const std::filesystem::path& study_path) const override {
    return version_;
  }

 private:
  int version_;
};
using namespace ProblemGenerationLog;
ProblemGenerationLoggerSharedPointer Getlog(
    const fs::path& tmp_directory_path_) {
  /**/
  auto logFile = std::make_shared<ProblemGenerationFileLogger>(
      tmp_directory_path_ / "ProblemGenerationLog.txt");

  auto logStd = std::make_shared<ProblemGenerationOstreamLogger>(std::cout);

  auto logger = std::make_shared<ProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO);
  logger->AddLogger(logFile);
  logger->AddLogger(logStd);
  return logger;

  /**/
}
class UpdateCapacitiesTest : public ::testing::Test {
 protected:
  void SetUp() {
    tmp_directory_path_ = std::tmpnam(nullptr);
    fs::create_directories(tmp_directory_path_ / "input" / "links" / "area1");
    ntc_path_ = tmp_directory_path_ / "input" / "links" / "area1" / "area2.txt";
    logger = Getlog(tmp_directory_path_);
    study_updater_ = StudyUpdater(tmp_directory_path_,
                                  AntaresVersionProviderStub(800), logger);
  }

  fs::path tmp_directory_path_;
  fs::path ntc_path_;

  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger;
  StudyUpdater study_updater_{fs::path("."), AntaresVersionProviderStub(800),
                              logger};
  AntaresLinkDataReader antares_link_data_reader_;
};

TEST_F(UpdateCapacitiesTest, update_nothing) {
  std::ofstream ntc_file;
  ntc_file.open(ntc_path_);
  for (auto i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    WriteRecord(ntc_file, LinkdataRecord({0.5, 0.3, 0, 0, 0, 0, 0, 0}));
  }
  ntc_file.close();
  (void)study_updater_.update(std::vector<ActiveLink>(),
                              std::map<std::string, double>());

  auto res = antares_link_data_reader_.Read(ntc_path_);
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(0.5, res.at(i).fileColumns.directCapacity_);
    ASSERT_EQ(0.3, res.at(i).fileColumns.indirectCapacity_);
  }
}

TEST_F(UpdateCapacitiesTest, update_one_link_no_candidate) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 1, logger);
  std::map<std::string, double> solution{
      {"dummy_link", 2},
  };
  (void)study_updater_.update({active_link}, solution);

  auto res =
      antares_link_data_reader_.Read(ntc_path_);  // Refactor NTC reader maybe ?
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(1, res.at(i).fileColumns.directCapacity_);
    ASSERT_EQ(1, res.at(i).fileColumns.indirectCapacity_);
  }
}

TEST_F(UpdateCapacitiesTest, update_one_link_one_candidate) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true, "dummy_link", 0, "area1",          "area2",
                          "",   "",           1, "dummy_candidate"};
  LinkProfile profile(logger);
  active_link.addCandidate(candidate, {profile});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };
  (void)study_updater_.update({active_link}, solution);

  auto res =
      antares_link_data_reader_.Read(ntc_path_);  // Refactor NTC reader maybe ?
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(400, res.at(i).fileColumns.directCapacity_);
    ASSERT_EQ(400, res.at(i).fileColumns.indirectCapacity_);
  }
}

TEST_F(UpdateCapacitiesTest, update_version_720) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true, "dummy_link", 0, "area1",          "area2",
                          "",   "",           1, "dummy_candidate"};
  LinkProfile profile(logger);
  active_link.addCandidate(candidate, {profile});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };

  study_updater_ = StudyUpdater(tmp_directory_path_,
                                AntaresVersionProviderStub(720), logger);
  (void)study_updater_.update({active_link}, solution);

  auto res =
      antares_link_data_reader_.Read(ntc_path_);  // Refactor NTC reader maybe ?
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(400, res.at(i).fileColumns.directCapacity_);
    ASSERT_EQ(400, res.at(i).fileColumns.indirectCapacity_);
  }
}

TEST_F(UpdateCapacitiesTest, update_link_parameters_version_720) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true, "dummy_link", 0, "area1",          "area2",
                          "",   "",           1, "dummy_candidate"};
  LinkProfile profile(logger);
  active_link.addCandidate(candidate, {profile});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };

  std::ofstream ntc_file;
  ntc_file.open(ntc_path_);
  for (auto i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    LinkdataRecord record;
    record.fileColumns = {0.5, 0.3, 4, 5, 6, 7, 8, 9};
    WriteRecord(ntc_file, record);
  }
  ntc_file.close();
  study_updater_ = StudyUpdater(tmp_directory_path_,
                                AntaresVersionProviderStub(720), logger);
  (void)study_updater_.update({active_link}, solution);

  auto res =
      antares_link_data_reader_.Read(ntc_path_);  // Refactor NTC reader maybe ?

  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(res.at(i).fileColumns.pShiftMin_, 8);
    ASSERT_EQ(res.at(i).fileColumns.pShiftMax_, 9);
    ASSERT_EQ(res.at(i).fileColumns.loopFlow_, 7);
    ASSERT_EQ(res.at(i).fileColumns.directHurdlesCost_, 4);
    ASSERT_EQ(res.at(i).fileColumns.indirectHurdlesCost_, 5);
    ASSERT_EQ(res.at(i).fileColumns.impedances_, 6);
  }
}

TEST_F(UpdateCapacitiesTest, update_version_800) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true, "dummy_link", 0, "area1",          "area2",
                          "",   "",           1, "dummy_candidate"};
  LinkProfile profile(logger);
  active_link.addCandidate(candidate, {profile});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };

  study_updater_ = StudyUpdater(tmp_directory_path_,
                                AntaresVersionProviderStub(811), logger);
  (void)study_updater_.update({active_link}, solution);

  auto res =
      antares_link_data_reader_.Read(ntc_path_);  // Refactor NTC reader maybe ?
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(400, res.at(i).fileColumns.directCapacity_);
    ASSERT_EQ(400, res.at(i).fileColumns.indirectCapacity_);
  }
}

TEST_F(UpdateCapacitiesTest, update_link_parameters_version_800) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true, "dummy_link", 0, "area1",          "area2",
                          "",   "",           1, "dummy_candidate"};
  LinkProfile profile(logger);
  active_link.addCandidate(candidate, {profile});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };

  std::ofstream ntc_file;
  ntc_file.open(ntc_path_);
  for (auto i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    LinkdataRecord record;
    record.fileColumns = {0.5, 0.3, 4, 5, 6, 7, 8, 9};
    WriteRecord(ntc_file, record);
  }
  ntc_file.close();
  study_updater_ = StudyUpdater(tmp_directory_path_,
                                AntaresVersionProviderStub(800), logger);
  (void)study_updater_.update({active_link}, solution);

  auto res =
      antares_link_data_reader_.Read(ntc_path_);  // Refactor NTC reader maybe ?

  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(res.at(i).fileColumns.pShiftMin_, 8);
    ASSERT_EQ(res.at(i).fileColumns.pShiftMax_, 9);
    ASSERT_EQ(res.at(i).fileColumns.loopFlow_, 7);
    ASSERT_EQ(res.at(i).fileColumns.directHurdlesCost_, 4);
    ASSERT_EQ(res.at(i).fileColumns.indirectHurdlesCost_, 5);
    ASSERT_EQ(res.at(i).fileColumns.impedances_, 6);
  }
}

TEST_F(UpdateCapacitiesTest,
       update_version_820_two_chronicle_installed_capacity_candidate_profile) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true,
                          "dummy_link",
                          0,
                          "area1",
                          "area2",
                          "candidate_installed_direct_profile",
                          "candidate_installed_indirect_profile",
                          1000,
                          "dummy_candidate",
                          0,
                          0,
                          0,
                          0,
                          "candidate_direct_profile",
                          "candidate_indirect_profile"};
  LinkProfile candidate_profile1(logger), candidate_profile2(logger);
  std::fill(candidate_profile1.direct_link_profile.begin(),
            candidate_profile1.direct_link_profile.end(), 0.2);
  std::fill(candidate_profile1.indirect_link_profile.begin(),
            candidate_profile1.indirect_link_profile.end(), 0.2);
  std::fill(candidate_profile2.direct_link_profile.begin(),
            candidate_profile2.direct_link_profile.end(), 0.5);
  std::fill(candidate_profile2.indirect_link_profile.begin(),
            candidate_profile2.indirect_link_profile.end(), 0.5);
  LinkProfile installed_chronicle1(logger), installed_chronicle2(logger);
  std::fill(installed_chronicle1.direct_link_profile.begin(),
            installed_chronicle1.direct_link_profile.end(), 0.1);
  std::fill(installed_chronicle1.indirect_link_profile.begin(),
            installed_chronicle1.indirect_link_profile.end(), 0.1);
  std::fill(installed_chronicle2.direct_link_profile.begin(),
            installed_chronicle2.direct_link_profile.end(), 0.9);
  std::fill(installed_chronicle2.indirect_link_profile.begin(),
            installed_chronicle2.indirect_link_profile.end(), 0.9);
  active_link.setAlreadyInstalledLinkProfiles(
      {installed_chronicle1, installed_chronicle2});

  active_link.addCandidate(candidate, {candidate_profile1, candidate_profile2});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };

  fs::create_directories(tmp_directory_path_ / "input" / "links" / "area1" /
                         "capacities");

  auto direct_ntc_file_path = tmp_directory_path_ / "input" / "links" /
                              "area1" / "capacities" / "area2_direct.txt";
  auto indirect_ntc_file_path = tmp_directory_path_ / "input" / "links" /
                                "area1" / "capacities" / "area2_indirect.txt";

  std::ofstream direct_file, indirect_file;
  direct_file.open(direct_ntc_file_path);
  indirect_file.open(indirect_ntc_file_path);
  for (auto i = 0; i < 8760; ++i) {
    direct_file << 0.5 << " " << 0.3 << "\n";
    indirect_file << 0.5 << " " << 0.3 << "\n";
  }
  direct_file.close();
  indirect_file.close();

  study_updater_ = StudyUpdater(tmp_directory_path_,
                                AntaresVersionProviderStub(822), logger);
  (void)study_updater_.update({active_link}, solution);

  auto profiles = LinkProfileReader(logger).ReadLinkProfile(
      direct_ntc_file_path,
      indirect_ntc_file_path);  // Refactor NTC reader maybe ?
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    /* Capacité du lien = capacité installée * profile installé
     *                  = 100 * 0.1 | 100 * 0.9
     *                  = 10 | 90
     * Investissement candidat N = investissement trouvé * profil candidat
     *                  = 300 * 0.2 | 300 * 0.5
     *                  = 60 | 150
     * Nouvelle capacité du lien = Capacité du lien + Somme(Investissement
     * candidat) = 70 | 240
     */
    ASSERT_EQ(70, profiles[0].getDirectProfile(i));
    ASSERT_EQ(70, profiles[0].getIndirectProfile(i));
    ASSERT_EQ(240, profiles[1].getDirectProfile(i));
    ASSERT_EQ(240, profiles[1].getIndirectProfile(i));
  }
}

TEST_F(UpdateCapacitiesTest, update_link_parameters_version_820) {
  ActiveLink active_link(0, "dummy_link", "area1", "area2", 100, logger);
  CandidateData candidate{true, "dummy_link", 0, "area1",          "area2",
                          "",   "",           1, "dummy_candidate"};
  LinkProfile profile(logger);
  active_link.addCandidate(candidate, {profile});
  std::map<std::string, double> solution{
      {"dummy_candidate", 300},
  };

  std::ofstream link_parameters;
  link_parameters.open(tmp_directory_path_ / "input" / "links" / "area1" /
                       "area2_parameters.txt");
  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    link_parameters << "4 5 6 7 8 9\n";
  }
  link_parameters.close();

  study_updater_ = StudyUpdater(tmp_directory_path_,
                                AntaresVersionProviderStub(820), logger);
  (void)study_updater_.update({active_link}, solution);

  auto res = antares_link_data_reader_.Read820(
      tmp_directory_path_ / "input" / "links" / "area1" /
      "area2_parameters.txt");  // Refactor NTC reader maybe ?

  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    ASSERT_EQ(res.at(i).fileColumns.pShiftMin_, 8);
    ASSERT_EQ(res.at(i).fileColumns.pShiftMax_, 9);
    ASSERT_EQ(res.at(i).fileColumns.loopFlow_, 7);
    ASSERT_EQ(res.at(i).fileColumns.directHurdlesCost_, 4);
    ASSERT_EQ(res.at(i).fileColumns.indirectHurdlesCost_, 5);
    ASSERT_EQ(res.at(i).fileColumns.impedances_, 6);
  }
}