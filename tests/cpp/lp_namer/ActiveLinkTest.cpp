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

void checkLinksProfiles(const std::vector<ActiveLink>& links, 
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

void checkLinksData(const std::vector<ActiveLink>& links, const std::vector<CandidateData>& candidatesDatas)
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

void checkLinks(const std::vector<ActiveLink>& links,
    const std::vector<CandidateData>& candidatesDatas,
    const std::unordered_map<std::string, std::vector<double>>& linkToReferenceDirectProfileMap,
    const std::unordered_map<std::string, std::vector<double>>& linkToReferenceIndirectProfileMap)
{
    checkLinksData(links, candidatesDatas);
    checkLinksProfiles(links, linkToReferenceDirectProfileMap, linkToReferenceIndirectProfileMap);
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

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
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
    
    checkLinks(links, cand_data_list, linkToReferenceDirectProfileMap, linkToReferenceIndirectProfileMap);
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

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    std::unordered_map<std::string, std::vector<double>> linkToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> linkToReferenceIndirectProfileMap;

    linkToReferenceDirectProfileMap[cand1.link] = directLinkprofile_l;
    linkToReferenceIndirectProfileMap[cand1.link] = indirectLinkprofile_l;

    std::unordered_map<std::string, std::vector<double>> candidateToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> candidateToReferenceIndirectProfileMap;

    candidateToReferenceDirectProfileMap[cand1.name] = directLinkprofile_l;
    candidateToReferenceIndirectProfileMap[cand1.name] = indirectLinkprofile_l;

    checkLinks(links, cand_data_list, linkToReferenceDirectProfileMap, linkToReferenceIndirectProfileMap);
    checkCandidates(links, cand_data_list, candidateToReferenceDirectProfileMap, candidateToReferenceIndirectProfileMap);

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

    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    std::unordered_map<std::string, std::vector<double>> linkToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> linkToReferenceIndirectProfileMap;

    linkToReferenceDirectProfileMap[cand1.link] = directLinkprofile_l;
    linkToReferenceIndirectProfileMap[cand1.link] = indirectLinkprofile_l;

    std::unordered_map<std::string, std::vector<double>> candidateToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> candidateToReferenceIndirectProfileMap;

    candidateToReferenceDirectProfileMap[cand1.name] = directLinkprofile_l;
    candidateToReferenceIndirectProfileMap[cand1.name] = indirectLinkprofile_l;
    candidateToReferenceDirectProfileMap[cand2.name] = directLinkprofile_l;
    candidateToReferenceIndirectProfileMap[cand2.name] = indirectLinkprofile_l;

    checkLinks(links, cand_data_list, linkToReferenceDirectProfileMap, linkToReferenceIndirectProfileMap);
    checkCandidates(links, cand_data_list, candidateToReferenceDirectProfileMap, candidateToReferenceIndirectProfileMap);

}

TEST(LinkBuilderTest, two_valid_candidates_data_on_two_different_link_no_profile_no_capacity)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";
    cand1.already_installed_capacity = 0;

    CandidateData cand2;
    cand2.link_id = 2;
    cand2.name = "pv";
    cand2.link = "area1 - pv";
    cand2.already_installed_capacity = 0;

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    std::vector<double> directLinkprofile_l(8760, 1);
    std::vector<double> indirectLinkprofile_l(8760, 1);

    std::unordered_map<std::string, std::vector<double>> linkToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> linkToReferenceIndirectProfileMap;

    linkToReferenceDirectProfileMap[cand1.link] = directLinkprofile_l;
    linkToReferenceIndirectProfileMap[cand1.link] = indirectLinkprofile_l;
    linkToReferenceDirectProfileMap[cand2.link] = directLinkprofile_l;
    linkToReferenceIndirectProfileMap[cand2.link] = indirectLinkprofile_l;

    std::unordered_map<std::string, std::vector<double>> candidateToReferenceDirectProfileMap;
    std::unordered_map<std::string, std::vector<double>> candidateToReferenceIndirectProfileMap;

    candidateToReferenceDirectProfileMap[cand1.name] = directLinkprofile_l;
    candidateToReferenceIndirectProfileMap[cand1.name] = indirectLinkprofile_l;
    candidateToReferenceDirectProfileMap[cand2.name] = directLinkprofile_l;
    candidateToReferenceIndirectProfileMap[cand2.name] = indirectLinkprofile_l;

    checkLinks(links, cand_data_list, linkToReferenceDirectProfileMap, linkToReferenceIndirectProfileMap);
    checkCandidates(links, cand_data_list, candidateToReferenceDirectProfileMap, candidateToReferenceIndirectProfileMap);

}


TEST(LinkBuilderTest, two_candidates_same_name_on_same_link)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";

    CandidateData cand2;
    cand2.link_id = 1;
    cand2.name = "transmission_line_1";
    cand2.link = "area1 - area2";

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

LinkProfile createProfile(std::vector<double>& directAlreadyInstalledLinkprofile_l, std::vector<double>& indirectAlreadyInstalledLinkprofile_l)
{
    LinkProfile profile;
    profile._directLinkProfile = directAlreadyInstalledLinkprofile_l;
    profile._indirectLinkProfile = indirectAlreadyInstalledLinkprofile_l;
    return profile;
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


TEST(LinkBuilderTest, two_candidates_with_same_name_on_two_different_links)
{
    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link = "area1 - area2";

    CandidateData cand2;
    cand2.link_id = 2;
    cand2.name = "transmission_line_1";
    cand2.link = "area2 - area3";

    std::vector<CandidateData> cand_data_list;
    cand_data_list.push_back(cand1);
    cand_data_list.push_back(cand2);

    std::map<std::string, LinkProfile> profile_map;

    try {
        ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
        FAIL() << "duplicate not detected";
    }
    catch (const std::runtime_error&  err) {
        ASSERT_STREQ(err.what(), "Candidate transmission_line_1 duplication detected");
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
