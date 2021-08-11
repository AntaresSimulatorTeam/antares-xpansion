#include "gtest/gtest.h"
#include "ActiveLinks.h"

const double DEFAULT_CAPACITY = 0;
const double DEFAULT_PROFILE_VALUE = 1;

LinkProfile createProfile(std::vector<double>& directAlreadyInstalledLinkprofile_l, std::vector<double>& indirectAlreadyInstalledLinkprofile_l)
{
    LinkProfile profile;
    profile._directLinkProfile = directAlreadyInstalledLinkprofile_l;
    profile._indirectLinkProfile = indirectAlreadyInstalledLinkprofile_l;
    return profile;
}

TEST(LinkBuilderTest, one_valid_candidate_no_profile_no_capacity)
{
    CandidateData cand1;
    cand1.link_id =1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = 0;
    
    std::vector<CandidateData> cand_data_list = {cand1};

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_EQ(links.size(), 1);
    ASSERT_EQ(links[0]._idInterco, 1);
    ASSERT_EQ(links[0]._name, "area1 - area2");
    ASSERT_DOUBLE_EQ(links[0]._already_installed_capacity, DEFAULT_CAPACITY);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
    }
    
    const auto& candidates = links[0].getCandidates();
    ASSERT_EQ(candidates.size(), 1);
    ASSERT_EQ(candidates[0]._name, "transmission_line_1");
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates[0].direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates[0].indirect_profile(timeStep), 1);
    }
}

TEST(LinkBuilderTest, one_valid_candidate_no_profile_with_capacity)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = 20;

    std::vector<CandidateData> cand_data_list = { cand1 };

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_DOUBLE_EQ(links[0]._already_installed_capacity, 20);

}

TEST(LinkBuilderTest, one_valid_candidate_with_profile_no_capacity)
{
    std::string link_profile_cand1 = "link_profile_cand1.txt";

    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.link_profile = link_profile_cand1;
    cand1.already_installed_capacity = 0;

    std::vector<CandidateData> cand_data_list = { cand1 };

    std::map<std::string, LinkProfile> profile_map;

    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    directLinkprofile_l[0] = 0;
    directLinkprofile_l[1] = 0.5;
    indirectLinkprofile_l[0] = 0.25;
    indirectLinkprofile_l[1] = 0.75;

    const auto linkProfileCandidat1 = createProfile(directLinkprofile_l, indirectLinkprofile_l);

    profile_map[link_profile_cand1] = linkProfileCandidat1;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_EQ(links.size(), 1);
    ASSERT_EQ(links[0]._idInterco, 1);
    ASSERT_EQ(links[0]._name, "area1 - area2");
    ASSERT_DOUBLE_EQ(links[0]._already_installed_capacity, DEFAULT_CAPACITY);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
    }

    const auto& candidates = links[0].getCandidates();
    ASSERT_EQ(candidates.size(), 1);
    ASSERT_EQ(candidates[0]._name, "transmission_line_1");
    ASSERT_EQ(candidates[0].direct_profile(0), 0);
    ASSERT_EQ(candidates[0].direct_profile(1), 0.5);
    ASSERT_EQ(candidates[0].indirect_profile(0), 0.25);
    ASSERT_EQ(candidates[0].indirect_profile(1), 0.75);
    for (int timeStep = 2; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates[0].direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates[0].indirect_profile(timeStep), 1);
    }
}

TEST(LinkBuilderTest, two_valid_candidate_no_profile_with_capacity)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = 20;

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_2";
    cand2.link = "area1 - area2";
    cand2.already_installed_capacity = 20;

    std::vector<CandidateData> cand_data_list = { cand1, cand2 };

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_EQ(links.size(), 1);
    ASSERT_EQ(links[0]._idInterco, 1);
    ASSERT_EQ(links[0]._name, "area1 - area2");
    ASSERT_DOUBLE_EQ(links[0]._already_installed_capacity, 20);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
    }

    const auto& candidates = links[0].getCandidates();
    ASSERT_EQ(candidates.size(), 2);

    ASSERT_EQ(candidates[0]._name, "transmission_line_1");
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates[0].direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates[0].indirect_profile(timeStep), 1);
    }

    ASSERT_EQ(candidates[1]._name, "transmission_line_2");
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates[1].direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates[1].indirect_profile(timeStep), 1);
    }

}

TEST(LinkBuilderTest, two_valid_candidates_data_on_two_different_link_no_profile_no_capacity)
{
    const double installed_capacity_link_0 = 0;
    const double installed_capacity_link_1 = 0;

    CandidateData cand1;
    cand1.link_id = 11;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = installed_capacity_link_0;

    CandidateData cand2;
    cand2.link_id = 12;
    cand2.name = "pv";
    cand2.link = "area1 - pv";
    cand2.already_installed_capacity = installed_capacity_link_1;

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_EQ(links.size(), 2);
    ASSERT_EQ(links[0]._idInterco, 11);
    ASSERT_EQ(links[0]._name, "area1 - area2");
    ASSERT_DOUBLE_EQ(links[0]._already_installed_capacity, installed_capacity_link_0);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(links[0].already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(links[0].already_installed_indirect_profile(timeStep), 1);
    }

    const auto& candidatesLink0 = links[0].getCandidates();
    ASSERT_EQ(candidatesLink0.size(), 1);

    ASSERT_EQ(candidatesLink0[0]._name, "transmission_line_1");
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidatesLink0[0].direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidatesLink0[0].indirect_profile(timeStep), 1);
    }

    ASSERT_EQ(links[1]._idInterco, 12);
    ASSERT_EQ(links[1]._name, "area1 - pv");
    ASSERT_DOUBLE_EQ(links[1]._already_installed_capacity, installed_capacity_link_1);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(links[1].already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(links[1].already_installed_indirect_profile(timeStep), 1);
    }

    const auto& candidatesLink1 = links[1].getCandidates();
    ASSERT_EQ(candidatesLink1.size(), 1);

    ASSERT_EQ(candidatesLink1[0]._name, "pv");
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidatesLink1[0].direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidatesLink1[0].indirect_profile(timeStep), 1);
    }

}


TEST(LinkBuilderTest, two_candidates_same_name)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";

    CandidateData cand2;
    cand2.link_id = 2;
    cand2.name = "transmission_line_1";
    cand2.link = "area1 - area3";

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);
    std::map<std::string, LinkProfile> profile_map;

    try {
        ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
        FAIL() << "duplicate not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "Candidate transmission_line_1 duplication detected");
    }

}

TEST(LinkBuilderTest, one_link_two_already_installed_profile)
{
    std::string temp_already_installed_profile1_name = "temp_already_installed_profile1.txt";
    std::string temp_already_installed_profile2_name = "temp_already_installed_profile2.txt";

    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = 0;
    cand1.already_installed_link_profile = temp_already_installed_profile1_name;

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_2";
    cand2.link = "area1 - area2";
    cand2.already_installed_capacity = 0;
    cand2.already_installed_link_profile = temp_already_installed_profile2_name;

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    directLinkprofile_l[0] = 0;
    directLinkprofile_l[1] = 0.5;
    indirectLinkprofile_l[0] = 0.25;
    indirectLinkprofile_l[1] = 0.75;

    const auto alreadyInstalledProfile = createProfile(directLinkprofile_l, indirectLinkprofile_l);

    std::map<std::string, LinkProfile> profile_map;

    profile_map[temp_already_installed_profile1_name] = alreadyInstalledProfile;
    profile_map[temp_already_installed_profile2_name] = alreadyInstalledProfile;

    try {
        ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
        FAIL() << "Candidate of the same links have different already_installed_links_profile and it's not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "Multiple already_installed_profile detected for link area1 - area2");
    }

}


TEST(LinkBuilderTest, one_link_with_two_different_already_installed_capacity)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = 20;

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_2";
    cand2.link = "area1 - area2";
    cand2.already_installed_capacity = 30;

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    try {
        ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
        FAIL() << "Same alreadyInstalledCapacity : not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "Multiple already installed capacity detected for link area1 - area2");
    }
}

TEST(LinkBuilderTest, missing_link_profile_in_profile_map)
{
    std::string cand_profile1_name = "cand1Profile.txt";
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.link_profile = cand_profile1_name;
    cand1.already_installed_capacity = 20;

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);

    std::map<std::string, LinkProfile> profile_map;

    try {
        ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
        FAIL() << "Missing link profile : not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "There is no linkProfile associated with cand1Profile.txt");
    }
}
