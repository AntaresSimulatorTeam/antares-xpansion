
#ifndef ANTARESXPANSION_ACTIVELINKS_H
#define ANTARESXPANSION_ACTIVELINKS_H

#include <Candidate.h>
#include <unordered_map>

class ActiveLink {

public:
	ActiveLink(int idInterco, const std::string linkName);
	void setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile);

	void addCandidate(const CandidateData& candidate_data, const LinkProfile& candidate_profile);
	const std::vector<Candidate>& getCandidates() const;
	
	double already_installed_direct_profile(size_t timeStep) const;
	double already_installed_indirect_profile(size_t timeStep) const;

public:
	int _idInterco;
	std::string _name;
	double _already_installed_capacity;

private:
	LinkProfile _already_installed_profile;
	std::vector<Candidate> _candidates;
};

class ActiveLinksBuilder {

public:

    ActiveLinksBuilder(const std::vector<CandidateData>& candidateList, const std::map<std::string, LinkProfile>& profile_map);
	
	const std::vector<ActiveLink>& getLinks();

private:
	void checkCandidateNameDuplication();
	void checkLinksValidity();

	int getIndexOf(int link_id) const;
	void addCandidate(const CandidateData &candidate_data);
	void launchExceptionIfNoLinkProfileAssociated(const std::string& profileName);

    std::unordered_map<std::string, std::string> linkToAlreadyInstalledProfileName;
	std::unordered_map<std::string, double> linkToAlreadyInstalledCapacity;
	const std::vector<CandidateData> _candidateDatas;
	const std::map<std::string, LinkProfile> _profile_map;
    std::vector <ActiveLink> _links;

    using linkName = std::string;
    struct LinkData{
        int id;
        double installed_capacity;
        std::string profile_name;
    };
    std::map<linkName, LinkData> _links_data;

    void record_link_data(const CandidateData &candidateData);

    void raise_errors_if_link_data_differs_from_existing_link(const LinkData &link_data,
                                                              const linkName &link_name) const;

    void create_links();

    LinkProfile getProfileFromProfileMap(const std::string &profile_name) const;
};

#endif //ANTARESXPANSION_ACTIVELINKS_H
