#include <fstream>

#include "antares-xpansion/lpnamer/input_reader/LinkProfileReader.h"
#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "gtest/gtest.h"

const std::string VALID_DIRECT_PROFILE_NAME("temp_direct_profile.txt");
const std::string VALID_INDIRECT_PROFILE_NAME("temp_indirect_profile.txt");
const std::string INVALID_DIRECT_PROFILE("temp_invalid_direct_profile.txt");

class LinkProfileReaderTest : public ::testing::Test {
 protected:
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_ =
      emptyLogger();
  static void createMergedProfileFile(
      const std::string& temp_profile_name,
      std::vector<double>& directLinkprofile_l,
      std::vector<double>& indirectLinkprofile_l) {
    std::ofstream file_profile(temp_profile_name);

    for (auto cnt_l = 0; cnt_l < directLinkprofile_l.size() - 1; ++cnt_l) {
      file_profile << directLinkprofile_l[cnt_l] << "\t"
                   << indirectLinkprofile_l[cnt_l] << "\n";
    }
    file_profile << directLinkprofile_l.back() << "\t"
                 << indirectLinkprofile_l.back();
    file_profile.close();
  }

  static void createProfileFile(const std::string& temp_profile_name,
                                std::vector<double>& linkprofile_l) {
    std::ofstream file_profile(temp_profile_name);

    for (auto cnt_l = 0; cnt_l < linkprofile_l.size() - 1; ++cnt_l) {
      file_profile << linkprofile_l[cnt_l] << "\n";
    }
    file_profile << linkprofile_l.back();
    file_profile.close();
  }

  static void SetUpTestCase() {
    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    directLinkprofile_l[0] = 0;
    directLinkprofile_l[1] = 0.5;
    indirectLinkprofile_l[0] = 0.25;
    indirectLinkprofile_l[1] = 0.75;

    createProfileFile(VALID_DIRECT_PROFILE_NAME, directLinkprofile_l);
    createProfileFile(VALID_INDIRECT_PROFILE_NAME, indirectLinkprofile_l);

    std::vector<double> invalid_directLinkprofile_l(100, 1);
    std::vector<double> invalid_indirectLinkprofile_l(100, 1);
    createMergedProfileFile(INVALID_DIRECT_PROFILE, invalid_directLinkprofile_l,
                            invalid_directLinkprofile_l);
  }

  static void TearDownTestCase() {
    // called after last test

    // delete the created tmp file
    std::remove(VALID_DIRECT_PROFILE_NAME.c_str());
    std::remove(INVALID_DIRECT_PROFILE.c_str());
  }

  void SetUp() {
    // called before each test
  }

  void TearDown() {
    // called after each test
  }
};

TEST_F(LinkProfileReaderTest, ReadValidSplitProfile) {
  LinkProfile profile =
      LinkProfileReader(logger_)
          .ReadLinkProfile(std::filesystem::path(VALID_DIRECT_PROFILE_NAME),
                           std::filesystem::path(VALID_INDIRECT_PROFILE_NAME))
          .at(0);

  ASSERT_EQ(profile.getDirectProfile(0), 0);
  ASSERT_EQ(profile.getIndirectProfile(0), 0.25);
  ASSERT_EQ(profile.getDirectProfile(1), 0.5);
  ASSERT_EQ(profile.getIndirectProfile(1), 0.75);
}

TEST_F(LinkProfileReaderTest, ReadOnlyDirectProfile) {
  LinkProfile profile = LinkProfileReader(logger_)
                            .ReadLinkProfile(VALID_DIRECT_PROFILE_NAME)
                            .at(0);

  ASSERT_EQ(profile.getDirectProfile(0), 0);
  ASSERT_EQ(profile.getIndirectProfile(0), 0);
  ASSERT_EQ(profile.getDirectProfile(1), 0.5);
  ASSERT_EQ(profile.getIndirectProfile(1), 0.5);
}

TEST_F(LinkProfileReaderTest, ReadInvalidMergedProfile) {
  try {
    [[maybe_unused]] LinkProfile profile =
        LinkProfileReader(logger_)
            .ReadLinkProfile(INVALID_DIRECT_PROFILE)
            .at(0);
    FAIL();
  } catch (const std::domain_error& expected) {
    ASSERT_STREQ(expected.what(), ("error not enough line in link-profile " +
                                   INVALID_DIRECT_PROFILE)
                                      .c_str());
  }
}

TEST_F(LinkProfileReaderTest, GetTimeStepLargerThan8760InSplitFile) {
  LinkProfile profile =
      LinkProfileReader(logger_)
          .ReadLinkProfile(std::filesystem::path(VALID_DIRECT_PROFILE_NAME),
                           std::filesystem::path(VALID_INDIRECT_PROFILE_NAME))
          .at(0);

  try {
    profile.getDirectProfile(8790);
    FAIL();
  } catch (const LinkProfile::InvalidHourForProfile& expected) {
    ASSERT_STREQ(expected.ErrorMessage().c_str(),
                 "Link profiles can be requested between point 0 and 8759.");
  }

  try {
    profile.getIndirectProfile(8790);
    FAIL();
  } catch (const LinkProfile::InvalidHourForProfile& expected) {
    ASSERT_STREQ(expected.ErrorMessage().c_str(),
                 "Link profiles can be requested between point 0 and 8759.");
  }
}

TEST_F(LinkProfileReaderTest, MapIsEmptyIfNoCandidateAsAProfile) {
  CandidateData candidate;
  candidate.installed_direct_link_profile_name = "";
  candidate.installed_indirect_link_profile_name = "";
  candidate.direct_link_profile = "";
  ASSERT_TRUE(
      LinkProfileReader(logger_).getLinkProfileMap({}, {candidate}).empty());
}

TEST_F(LinkProfileReaderTest, MapContainLinkProfileOfCandidate) {
  CandidateData candidate;
  candidate.installed_direct_link_profile_name = "";
  candidate.installed_indirect_link_profile_name = "";
  candidate.direct_link_profile = VALID_DIRECT_PROFILE_NAME;
  candidate.indirect_link_profile = VALID_INDIRECT_PROFILE_NAME;

  auto profiles = LinkProfileReader(logger_).getLinkProfileMap({}, {candidate});

  auto profile = profiles.at(VALID_DIRECT_PROFILE_NAME).at(0);
  ASSERT_EQ(profile.getDirectProfile(0), 0);
  ASSERT_EQ(profile.getIndirectProfile(0), 0.25);
  ASSERT_EQ(profile.getDirectProfile(1), 0.5);
  ASSERT_EQ(profile.getIndirectProfile(1), 0.75);
}

TEST_F(LinkProfileReaderTest, MapContainInstalledProfileOfCandidate) {
  CandidateData candidate;
  candidate.installed_direct_link_profile_name = VALID_DIRECT_PROFILE_NAME;
  candidate.installed_indirect_link_profile_name = VALID_DIRECT_PROFILE_NAME;

  auto profiles = LinkProfileReader(logger_).getLinkProfileMap({}, {candidate});

  auto profile = profiles.at(VALID_DIRECT_PROFILE_NAME).at(0);
  ASSERT_EQ(profile.getDirectProfile(0), 0);
  ASSERT_EQ(profile.getIndirectProfile(0), 0);
  ASSERT_EQ(profile.getDirectProfile(1), 0.5);
  ASSERT_EQ(profile.getIndirectProfile(1), 0.5);
}