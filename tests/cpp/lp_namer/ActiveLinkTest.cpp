#include "gtest/gtest.h"
#include "ActiveLinks.h"
#include <unordered_set>
#include <unordered_map>

struct LinkRef {
    LinkRef(int id, const std::string& name, double already_installed_capacity)
        :_id(id),_name(name), _already_installed_capacity(already_installed_capacity){};
    LinkRef() {};
    int _id;
    std::string _name;
    double _already_installed_capacity;
};

void checkLinkProfile(const std::vector<ActiveLink>& links, 
                  const std::unordered_map<std::string, std::vector<double>>& linkToReferenceDirectProfileMap,
                  const std::unordered_map<std::string, std::vector<double>>& linkToReferenceIndirectProfileMap)
{
    ASSERT_EQ(links.size(), linkToReferenceDirectProfileMap.size());
    ASSERT_EQ(links.size(), linkToReferenceIndirectProfileMap.size());

    for (const auto& link : links)
    {
        ASSERT_EQ(linkToReferenceDirectProfileMap.at(link._name).size(), 8760);
        ASSERT_EQ(linkToReferenceIndirectProfileMap.at(link._name).size(), 8760);
        for (int timeStep = 0; timeStep < 8760; timeStep++)
        {
            ASSERT_DOUBLE_EQ(link.already_installed_direct_profile(timeStep), linkToReferenceDirectProfileMap.at(link._name).at(timeStep));
            ASSERT_DOUBLE_EQ(link.already_installed_indirect_profile(timeStep), linkToReferenceIndirectProfileMap.at(link._name).at(timeStep));
        }
    }
}

void checkLinksName(const std::vector<ActiveLink>& links, const std::vector<CandidateData>& candidatesDatas)
{
    std::unordered_map<std::string, LinkRef> linkNameToLinkDataRefMap;

    for (const auto& candidateData : candidatesDatas)
    {
        if (linkNameToLinkDataRefMap.find(candidateData.link) == linkNameToLinkDataRefMap.end())
        {
            LinkRef linkRef(candidateData.link_id, candidateData.link, candidateData.already_installed_capacity);
            
            linkNameToLinkDataRefMap[candidateData.link] = linkRef;
        }
    }
    ASSERT_EQ(links.size(), linkNameToLinkDataRefMap.size());

    for (int indexLink = 0; indexLink < links.size(); indexLink++)
    {
        const auto& link = links.at(indexLink);
        ASSERT_TRUE(linkNameToLinkDataRefMap.find(link._name) != linkNameToLinkDataRefMap.end());
        ASSERT_EQ(linkNameToLinkDataRefMap.at(link._name)._id, link._idInterco);
        ASSERT_EQ(linkNameToLinkDataRefMap.at(link._name)._already_installed_capacity, link._already_installed_capacity);
    }
}

void checkCandidates(const std::vector<ActiveLink>& links,
                     const std::vector<CandidateData>& candidatesDatas,
                     const std::unordered_map<std::string, std::vector<double>>& CandidateToReferenceDirectProfileMap,
                     const std::unordered_map<std::string, std::vector<double>>& CandidateToReferenceIndirectProfileMap
    )
{
    std::unordered_map<std::string, Candidate> candidatesMap;
    
    for (const auto& link : links)
    {
        const std::vector<Candidate>& candidates = link.getCandidates();
        for (const auto& candidate : candidates)
        {
            candidatesMap[candidate._name] = candidate;
        }

    }
    ASSERT_EQ(candidatesDatas.size(), candidatesMap.size());
    ASSERT_EQ(candidatesDatas.size(), CandidateToReferenceDirectProfileMap.size());
    ASSERT_EQ(candidatesDatas.size(), CandidateToReferenceIndirectProfileMap.size());

    for (const auto& candidateData : candidatesDatas)
    {
        ASSERT_TRUE(candidatesMap.find(candidateData.name) != candidatesMap.end());
        const auto& candidate = candidatesMap.at(candidateData.name);
        
        ASSERT_EQ(CandidateToReferenceDirectProfileMap.at(candidate._name).size(), 8760);
        ASSERT_EQ(CandidateToReferenceIndirectProfileMap.at(candidate._name).size(), 8760);
        for (int timeStep = 0; timeStep < 8760; timeStep++)
        {
            ASSERT_DOUBLE_EQ(candidate.direct_profile(timeStep), CandidateToReferenceDirectProfileMap.at(candidate._name).at(timeStep));
            ASSERT_DOUBLE_EQ(candidate.indirect_profile(timeStep), CandidateToReferenceIndirectProfileMap.at(candidate._name).at(timeStep));
        }
    }
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

    ActiveLinks linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    std::unordered_map<std::string, std::vector<double>> linkToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> linkToReferenceIndirectProfileMap;

    linkToReferenceDirectProfileMap[cand1.link]     = directLinkprofile_l;
    linkToReferenceIndirectProfileMap[cand1.link]   = indirectLinkprofile_l;

    std::unordered_map<std::string, std::vector<double>> candidateToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> candidateToReferenceIndirectProfileMap;

    candidateToReferenceDirectProfileMap[cand1.name]    = directLinkprofile_l;
    candidateToReferenceIndirectProfileMap[cand1.name]  = indirectLinkprofile_l;
  
    checkLinksName(links, cand_data_list);
    checkLinkProfile(links, linkToReferenceDirectProfileMap, linkToReferenceIndirectProfileMap);
    checkCandidates(links, cand_data_list, candidateToReferenceDirectProfileMap, candidateToReferenceIndirectProfileMap);
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

    ActiveLinks linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_EQ(links.size(), 1);

    const ActiveLink& link = links.at(0);

    ASSERT_EQ(link.getId(), cand1.link_id);
    ASSERT_EQ(link._name, cand1.link);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(link.already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(link.already_installed_indirect_profile(timeStep), 1);
    }
    ASSERT_DOUBLE_EQ(link._already_installed_capacity, 20);

    const std::vector<Candidate>& candidates = link.getCandidates();
    ASSERT_EQ(candidates.size(), 1);
    ASSERT_EQ(candidates.at(0)._name, cand1.name);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates.at(0).direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates.at(0).indirect_profile(timeStep), 1);
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

    ActiveLinks linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    ASSERT_EQ(links.size(), 1);

    const ActiveLink& link = links.at(0);

    ASSERT_EQ(link.getId(), cand1.link_id);
    ASSERT_EQ(link._name, cand1.link);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(link.already_installed_direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(link.already_installed_indirect_profile(timeStep), 1);
    }
    ASSERT_DOUBLE_EQ(link._already_installed_capacity, 20);

    const std::vector<Candidate>& candidates = link.getCandidates();
    ASSERT_EQ(candidates.size(), 2);
    ASSERT_EQ(candidates.at(0)._name, cand_data_list[0].name);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates.at(0).direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates.at(0).indirect_profile(timeStep), 1);
    }
    ASSERT_EQ(candidates.at(1)._name, cand_data_list[1].name);
    for (int timeStep = 0; timeStep < 8760; timeStep++)
    {
        ASSERT_DOUBLE_EQ(candidates.at(1).direct_profile(timeStep), 1);
        ASSERT_DOUBLE_EQ(candidates.at(1).indirect_profile(timeStep), 1);
    }

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

    ActiveLinks linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

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

    ActiveLinks linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

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
        ActiveLinks linkBuilder{ cand_data_list, profile_map };
        const std::vector<ActiveLink>& links = linkBuilder.getLinks();
        FAIL() << "duplicate not detected";
    }
    catch (const std::runtime_error& err) {
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
        ActiveLinks linkBuilder{ cand_data_list, profile_map };
        const std::vector<ActiveLink>& links = linkBuilder.getLinks();
        FAIL() << "Candidate of the same links have different already_installed_links_profile and it's not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "Multiple already_installed_profile detected for link area1 - area2");
    }

}


TEST(ActiveLinkTests, same_candidate_name_on_two_links)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.linkor = "area1";
    cand1.linkex = "area2";

    CandidateData cand2;
    cand2.link_id = 2;
    cand2.name = "transmission_line_1";
    cand2.link = "area2 - area3";
    cand2.linkor = "area2";
    cand2.linkex = "area3";

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    try {
        ActiveLinks linkBuilder{ cand_data_list, profile_map };
        const std::vector<ActiveLink>& links = linkBuilder.getLinks();
        FAIL() << "duplicate not detected";
    }
    catch (const std::runtime_error&  err) {
        ASSERT_STREQ(err.what(), "Candidate transmission_line_1 duplication detected");
    }
}

TEST(ActiveLinkTests, DISABLED_different_already_installed_capacity_for_two_candidate_on_same_link)
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
        ActiveLinks linkBuilder{ cand_data_list, profile_map };
        const std::vector<ActiveLink>& links = linkBuilder.getLinks();
        FAIL() << "Same alreadyInstalledCapacity : not detected";
    }
    catch (const std::runtime_error& err) {
        ASSERT_STREQ(err.what(), "Multiple already installed capacity detected for link area1 - area2");
    }
}
