#include "gtest/gtest.h"
#include "ActiveLinks.h"


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
