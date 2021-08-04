#include "gtest/gtest.h"
#include "ActiveLinks.h"


TEST(ActiveLinkTests, valid_candidate_data)
{
    CandidateData cand;
    cand.link_id ==1;

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand);

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinks links = ActiveLinksInitializer().createActiveLinkFromCandidates(cand_data_list, profile_map);

    ASSERT_EQ(links.getLink().size(), 1);

}

