#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "gtest/gtest.h"

const double DEFAULT_CAPACITY = 0;
const double DEFAULT_PROFILE_VALUE = 1;

static ProblemGenerationLog::ProblemGenerationLoggerSharedPointer
emptyLogger() {
  return std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(
      LogUtils::LOGLEVEL::NONE);
}
LinkProfile createProfile(
    const std::vector<double>& directAlreadyInstalledLinkprofile_l,
    const std::vector<double>& indirectAlreadyInstalledLinkprofile_l) {
  LinkProfile profile(emptyLogger());
  profile.direct_link_profile = directAlreadyInstalledLinkprofile_l;
  profile.indirect_link_profile = indirectAlreadyInstalledLinkprofile_l;
  return profile;
}

TEST(LinkBuilderTest, one_valid_candidate_no_profile_no_capacity) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;
  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  ASSERT_EQ(links.size(), 1);
  ASSERT_EQ(links[0].get_idLink(), 1);
  ASSERT_EQ(links[0].get_LinkName(), "area1 - area2");
  ASSERT_DOUBLE_EQ(links[0].get_already_installed_capacity(), DEFAULT_CAPACITY);
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
    ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
  }

  const auto& candidates = links[0].getCandidates();
  ASSERT_EQ(candidates.size(), 1);
  ASSERT_EQ(candidates[0].get_name(), "transmission_line_1");
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(candidates[0].directCapacityFactor(timeStep), 1);
    ASSERT_DOUBLE_EQ(candidates[0].indirectCapacityFactor(timeStep), 1);
  }
}

TEST(LinkBuilderTest, one_valid_candidate_no_profile_with_capacity) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 20;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;
  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  ASSERT_DOUBLE_EQ(links[0].get_already_installed_capacity(), 20);
}

TEST(LinkBuilderTest, one_valid_candidate_with_profile_no_capacity) {
  std::string link_profile_cand1 = "link_profile_cand1.txt";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.direct_link_profile = link_profile_cand1;
  cand1.indirect_link_profile = link_profile_cand1;
  cand1.already_installed_capacity = 0;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  const auto linkProfileCandidat1 =
      createProfile(directLinkprofile_l, indirectLinkprofile_l);

  profile_map[link_profile_cand1] = {linkProfileCandidat1};

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  ASSERT_EQ(links.size(), 1);
  ASSERT_EQ(links[0].get_idLink(), 1);
  ASSERT_EQ(links[0].get_LinkName(), "area1 - area2");
  ASSERT_DOUBLE_EQ(links[0].get_already_installed_capacity(), DEFAULT_CAPACITY);
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
    ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
  }

  const auto& candidates = links[0].getCandidates();
  ASSERT_EQ(candidates.size(), 1);
  ASSERT_EQ(candidates[0].get_name(), "transmission_line_1");
  ASSERT_EQ(candidates[0].directCapacityFactor(0), 0);
  ASSERT_EQ(candidates[0].directCapacityFactor(1), 0.5);
  ASSERT_EQ(candidates[0].indirectCapacityFactor(0), 0.25);
  ASSERT_EQ(candidates[0].indirectCapacityFactor(1), 0.75);
  for (int timeStep = 2; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(candidates[0].directCapacityFactor(timeStep), 1);
    ASSERT_DOUBLE_EQ(candidates[0].indirectCapacityFactor(timeStep), 1);
  }
}

TEST(LinkBuilderTest, two_valid_candidate_no_profile_with_capacity) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 20;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 20;

  std::vector<CandidateData> cand_data_list = {cand1, cand2};

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  ASSERT_EQ(links.size(), 1);
  ASSERT_EQ(links[0].get_idLink(), 1);
  ASSERT_EQ(links[0].get_LinkName(), "area1 - area2");
  ASSERT_DOUBLE_EQ(links[0].get_already_installed_capacity(), 20);
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
    ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
  }

  const auto& candidates = links[0].getCandidates();
  ASSERT_EQ(candidates.size(), 2);

  ASSERT_EQ(candidates[0].get_name(), "transmission_line_1");
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(candidates[0].directCapacityFactor(timeStep), 1);
    ASSERT_DOUBLE_EQ(candidates[0].indirectCapacityFactor(timeStep), 1);
  }

  ASSERT_EQ(candidates[1].get_name(), "transmission_line_2");
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(candidates[1].directCapacityFactor(timeStep), 1);
    ASSERT_DOUBLE_EQ(candidates[1].indirectCapacityFactor(timeStep), 1);
  }
}

TEST(LinkBuilderTest,
     two_valid_candidates_data_on_two_different_link_no_profile_no_capacity) {
  const double installed_capacity_link_0 = 0;
  const double installed_capacity_link_1 = 0;

  CandidateData cand1;
  cand1.link_id = 11;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = installed_capacity_link_0;

  CandidateData cand2;
  cand2.link_id = 12;
  cand2.name = "pv";
  cand2.link_name = "area1 - pv";
  cand2.already_installed_capacity = installed_capacity_link_1;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  ASSERT_EQ(links.size(), 2);
  ASSERT_EQ(links[0].get_idLink(), 11);
  ASSERT_EQ(links[0].get_LinkName(), "area1 - area2");
  ASSERT_DOUBLE_EQ(links[0].get_already_installed_capacity(),
                   installed_capacity_link_0);
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
    ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
  }

  const auto& candidatesLink0 = links[0].getCandidates();
  ASSERT_EQ(candidatesLink0.size(), 1);

  ASSERT_EQ(candidatesLink0[0].get_name(), "transmission_line_1");
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(candidatesLink0[0].directCapacityFactor(timeStep), 1);
    ASSERT_DOUBLE_EQ(candidatesLink0[0].indirectCapacityFactor(timeStep), 1);
  }

  ASSERT_EQ(links[1].get_idLink(), 12);
  ASSERT_EQ(links[1].get_LinkName(), "area1 - pv");
  ASSERT_DOUBLE_EQ(links[1].get_already_installed_capacity(),
                   installed_capacity_link_1);
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(links[1].already_installed_direct_profile(timeStep), 1);
    ASSERT_DOUBLE_EQ(links[1].already_installed_indirect_profile(timeStep), 1);
  }

  const auto& candidatesLink1 = links[1].getCandidates();
  ASSERT_EQ(candidatesLink1.size(), 1);

  ASSERT_EQ(candidatesLink1[0].get_name(), "pv");
  for (int timeStep = 0; timeStep < 8760; timeStep++) {
    ASSERT_DOUBLE_EQ(candidatesLink1[0].directCapacityFactor(timeStep), 1);
    ASSERT_DOUBLE_EQ(candidatesLink1[0].indirectCapacityFactor(timeStep), 1);
  }
}

TEST(LinkBuilderTest,
     error_if_two_candidates_on_same_link_only_one_with_installed_profile) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.installed_direct_link_profile_name = "dummy";

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);
  std::map<std::string, std::vector<LinkProfile>> profile_map = {
      {"dummy", {LinkProfile(emptyLogger())}}};

  try {
    auto logger = emptyLogger();
    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
    FAIL() << "duplicate not detected";
  } catch (
      const ActiveLinksBuilder::MultipleAlreadyInstalledProfileDetectedForLink&
          err) {
    ASSERT_STREQ(
        err.ErrorMessage().c_str(),
        "Multiple already_installed_profile detected for link area1 - area2");
  }
}

TEST(LinkBuilderTest,
     error_if_two_candidates_on_same_link_with_different_linkid) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";

  CandidateData cand2;
  cand2.link_id = 2;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);
  std::map<std::string, std::vector<LinkProfile>> profile_map;

  try {
    auto logger = emptyLogger();
    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
    FAIL() << "incompatibility of link_id not detected";
  } catch (const ActiveLinksBuilder::MultipleLinkIddetectedForLink& err) {
    ASSERT_STREQ(err.ErrorMessage().c_str(),
                 "Multiple link_id detected for link area1 - area2");
  }
}

TEST(LinkBuilderTest, two_candidates_same_name) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";

  CandidateData cand2;
  cand2.link_id = 2;
  cand2.name = "transmission_line_1";
  cand2.link_name = "area1 - area3";

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);
  std::map<std::string, std::vector<LinkProfile>> profile_map;

  try {
    auto logger = emptyLogger();
    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
    FAIL() << "duplicate not detected";
  } catch (const ActiveLinksBuilder::CandidateDuplicationDetected& err) {
    ASSERT_STREQ(err.ErrorMessage().c_str(),
                 "Candidate transmission_line_1 duplication detected");
  }
}

TEST(LinkBuilderTest, one_link_two_already_installed_profile) {
  std::string temp_already_installed_profile1_name =
      "temp_already_installed_profile1.txt";
  std::string temp_already_installed_profile2_name =
      "temp_already_installed_profile2.txt";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 0;
  cand2.installed_direct_link_profile_name =
      temp_already_installed_profile2_name;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  const auto alreadyInstalledProfile =
      createProfile(directLinkprofile_l, indirectLinkprofile_l);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  profile_map[temp_already_installed_profile1_name] = {alreadyInstalledProfile};
  profile_map[temp_already_installed_profile2_name] = {alreadyInstalledProfile};

  try {
    auto logger = emptyLogger();
    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
    FAIL() << "Candidate of the same links have different "
              "already_installed_links_profile and it's not detected";
  } catch (
      const ActiveLinksBuilder::MultipleAlreadyInstalledProfileDetectedForLink&
          err) {
    ASSERT_STREQ(
        err.ErrorMessage().c_str(),
        "Multiple already_installed_profile detected for link area1 - area2");
  }
}

TEST(LinkBuilderTest,
     properly_set_direct_and_indirect_already_installed_profiles) {
  std::string temp_already_installed_profile1_name =
      "temp_already_installed_profile1.txt";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 0;
  cand2.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  const auto alreadyInstalledProfile =
      createProfile(directLinkprofile_l, indirectLinkprofile_l);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  profile_map[temp_already_installed_profile1_name] = {alreadyInstalledProfile};

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  auto links = linkBuilder.getLinks();
  ASSERT_EQ(links[0].already_installed_direct_profile(0), 0);
  ASSERT_EQ(links[0].already_installed_direct_profile(1), 0.5);
  ASSERT_EQ(links[0].already_installed_indirect_profile(0), 0.25);
  ASSERT_EQ(links[0].already_installed_indirect_profile(1), 0.75);
}

TEST(
    LinkBuilderTest,
    properly_set_direct_and_indirect_already_installed_profiles_diffrent_chronicle) {
  std::string temp_already_installed_profile1_name =
      "temp_already_installed_profile1.txt";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 0;
  cand2.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  const auto alreadyInstalledProfile =
      createProfile(directLinkprofile_l, indirectLinkprofile_l);

  std::map<std::string, std::vector<LinkProfile>> profile_map;
  auto logger = emptyLogger();
  profile_map[temp_already_installed_profile1_name] = {LinkProfile(logger),
                                                       alreadyInstalledProfile};

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  auto links = linkBuilder.getLinks();
  ASSERT_EQ(links[0].already_installed_direct_profile(2, 0), 0);
  ASSERT_EQ(links[0].already_installed_direct_profile(2, 1), 0.5);
  ASSERT_EQ(links[0].already_installed_indirect_profile(2, 0), 0.25);
  ASSERT_EQ(links[0].already_installed_indirect_profile(2, 1), 0.75);
}

TEST(LinkBuilderTest, return_first_profile_if_chronicle_does_not_exists) {
  std::string temp_already_installed_profile1_name =
      "temp_already_installed_profile1.txt";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 0;
  cand2.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  const auto alreadyInstalledProfile =
      createProfile(directLinkprofile_l, indirectLinkprofile_l);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  profile_map[temp_already_installed_profile1_name] = {{},
                                                       alreadyInstalledProfile};

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  auto links = linkBuilder.getLinks();
  ASSERT_EQ(links[0].already_installed_direct_profile(5, 0), 1);
  ASSERT_EQ(links[0].already_installed_direct_profile(5, 1), 1);
  ASSERT_EQ(links[0].already_installed_indirect_profile(5, 0), 1);
  ASSERT_EQ(links[0].already_installed_indirect_profile(5, 1), 1);
}

TEST(
    LinkBuilderTest,
    properly_access_missing_installed_profiles_with_correct_number_of_chronicles) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.direct_link_profile = "profile_cand_1";

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 0;
  cand2.direct_link_profile = "profile_cand_2";

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  profile_map[cand1.direct_link_profile] = {
      createProfile(directLinkprofile_l, indirectLinkprofile_l),
      createProfile(directLinkprofile_l, indirectLinkprofile_l),
      createProfile(directLinkprofile_l, indirectLinkprofile_l)};
  profile_map[cand2.direct_link_profile] = {
      createProfile(directLinkprofile_l, indirectLinkprofile_l),
      createProfile(directLinkprofile_l, indirectLinkprofile_l),
      createProfile(directLinkprofile_l, indirectLinkprofile_l)};

  // Verifier acces installed profile OK

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  auto links = linkBuilder.getLinks();
  ASSERT_EQ(links[0].already_installed_direct_profile(
                profile_map[cand1.direct_link_profile].size() - 1, 0),
            1);
  ASSERT_EQ(links[0].already_installed_indirect_profile(
                profile_map[cand1.direct_link_profile].size() - 1, 0),
            1);
}

TEST(
    LinkBuilderTest,
    properly_access_missing_candidate_profiles_with_correct_number_of_chronicles) {
  std::string temp_already_installed_profile1_name =
      "temp_already_installed_profile1.txt";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 0;
  cand2.installed_direct_link_profile_name =
      temp_already_installed_profile1_name;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::vector<double> directLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);
  std::vector<double> indirectLinkprofile_l(NUMBER_OF_HOUR_PER_YEAR, 1);

  directLinkprofile_l[0] = 0;
  directLinkprofile_l[1] = 0.5;
  indirectLinkprofile_l[0] = 0.25;
  indirectLinkprofile_l[1] = 0.75;

  const auto alreadyInstalledProfile =
      createProfile(directLinkprofile_l, indirectLinkprofile_l);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  profile_map[temp_already_installed_profile1_name] = {alreadyInstalledProfile,
                                                       alreadyInstalledProfile,
                                                       alreadyInstalledProfile};

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  auto links = linkBuilder.getLinks();
  for (auto const& candidate : links[0].getCandidates()) {
    ASSERT_EQ(
        candidate.directCapacityFactor(
            profile_map[temp_already_installed_profile1_name].size() - 1, 0),
        1);
    ASSERT_EQ(
        candidate.indirectCapacityFactor(
            profile_map[temp_already_installed_profile1_name].size() - 1, 0),
        1);
  }
}

TEST(LinkBuilderTest, one_link_with_two_different_already_installed_capacity) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 20;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.already_installed_capacity = 30;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  try {
    auto logger = emptyLogger();
    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
    FAIL() << "Same alreadyInstalledCapacity : not detected";
  } catch (
      const ActiveLinksBuilder::MultipleAlreadyInstalledCapacityDetectedForLink&
          err) {
    ASSERT_STREQ(
        err.ErrorMessage().c_str(),
        "Multiple already installed capacity detected for link area1 - area2");
  }
}

TEST(LinkBuilderTest, missing_link_profile_in_profile_map) {
  std::string cand_profile1_name = "cand1Profile.txt";
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.direct_link_profile = cand_profile1_name;
  cand1.indirect_link_profile = cand_profile1_name;
  cand1.already_installed_capacity = 20;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  try {
    auto logger = emptyLogger();
    ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
    FAIL() << "Missing link profile : not detected";
  } catch (
      const ActiveLinksBuilder::ThereIsNoLinkProfileAssociatedWithThisProfile&
          err) {
    ASSERT_STREQ(err.ErrorMessage().c_str(),
                 "There is no linkProfile associated with cand1Profile.txt");
  }
}

TEST(LinkBuilderTest, candidate_not_enable) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.enable = true;

  CandidateData cand2;
  cand2.link_id = 1;
  cand2.name = "transmission_line_2";
  cand2.link_name = "area1 - area2";
  cand2.enable = false;

  std::vector<CandidateData> cand_data_list;
  cand_data_list.push_back(cand1);
  cand_data_list.push_back(cand2);
  std::map<std::string, std::vector<LinkProfile>> profile_map;

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();
  const auto& candidates = links[0].getCandidates();
  ASSERT_EQ(candidates.size(), 1);
}

TEST(LinkBuilderTest, ChronicleMap_properly_loaded_in_link) {
  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.linkor = "area2";
  cand1.linkex = "area1";
  cand1.already_installed_capacity = 20;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  std::filesystem::path ts_info_root_ = std::filesystem::temp_directory_path();
  std::filesystem::create_directories(ts_info_root_ / "area1");
  std::ofstream b_file(ts_info_root_ / "area1" / "area2.txt");
  b_file << "Garbage\n52\n";
  b_file.close();

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{
      cand_data_list, profile_map,
      DirectAccessScenarioToChronicleProvider(ts_info_root_, logger), logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  std::map<unsigned, unsigned> expected = {{1, 52}};
  ASSERT_EQ(links[0].McYearToChronicle(), expected);
}