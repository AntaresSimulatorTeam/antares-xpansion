#include "gtest/gtest.h"
#include "ActiveLinks.h"
#include "ActiveLinkTest.h"


TEST(ActiveLinkTests, valid_candidate_data)
{
    CandidateData cand1;
    cand1.link_id =1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.linkor = "area1";
    cand1.linkex = "area2";
    
    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinks links = ActiveLinksInitializer().createActiveLinkFromCandidates(cand_data_list, profile_map);

    ASSERT_EQ(links.size(), 1);

}

TEST(ActiveLinkTests, multiple_valid_candidates_data_on_same_link)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.linkor = "area1";
    cand1.linkex = "area2";

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_2";
    cand2.link = "area1 - area2";
    cand2.linkor = "area1";
    cand2.linkex = "area2";

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinks links = ActiveLinksInitializer().createActiveLinkFromCandidates(cand_data_list, profile_map);

    ASSERT_EQ(links.size(), 1);

}

TEST(ActiveLinkTests, multiple_valid_candidates_data_on_different_link)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.linkor = "area1";
    cand1.linkex = "area2";

    CandidateData cand2;
    cand2.link_id = 2;
    cand2.name = "pv";
    cand2.link = "area1 - pv";
    cand2.linkor = "area1";
    cand2.linkex = "pv";

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinks links = ActiveLinksInitializer().createActiveLinkFromCandidates(cand_data_list, profile_map);

    ASSERT_EQ(links.size(), 2);

}

TEST(ActiveLinkTests, multiple_candidate_same_name_on_same_link)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.linkor = "area1";
    cand1.linkex = "area2";

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_1";
    cand2.link = "area1 - area2";
    cand2.linkor = "area1";
    cand2.linkex = "area2";

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    try {
        ActiveLinks links = ActiveLinksInitializer().createActiveLinkFromCandidates(cand_data_list, profile_map);
        FAIL() << "duplicate not detected";
    }
    catch (const std::runtime_error&  err) {
        ASSERT_STREQ(err.what(), "Candidate transmission_line_1 duplication detected");
    }

}

LinkProfile createProfile(std::vector<double>& directAlreadyInstalledLinkprofile_l, std::vector<double>& indirectAlreadyInstalledLinkprofile_l)
{
    LinkProfile profile;
    profile._directLinkProfile = directAlreadyInstalledLinkprofile_l;
    profile._indirectLinkProfile = indirectAlreadyInstalledLinkprofile_l;
    return profile;
}

TEST(ActiveLinkTests, multiple_candidate_different_already_installed_profile_on_same_link)
{
    std::string temp_already_installed_profile1_name = "temp_already_installed_profile1.txt";
    std::string temp_already_installed_profile2_name = "temp_already_installed_profile2.txt";

    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.linkor = "area1";
    cand1.linkex = "area2";
    cand1.already_installed_link_profile = temp_already_installed_profile1_name;

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_2";
    cand2.link = "area1 - area2";
    cand2.linkor = "area1";
    cand2.linkex = "area2";
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
        ActiveLinks links = ActiveLinksInitializer().createActiveLinkFromCandidates(cand_data_list, profile_map);
        FAIL() << "Candidate of the same links have different already_installed_links_profile and it's not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "Multiple already_installed_profile detected for link area1 - area2");
    }

}
