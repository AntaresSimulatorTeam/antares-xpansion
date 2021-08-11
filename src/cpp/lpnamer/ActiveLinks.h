
#ifndef ANTARESXPANSION_ACTIVELINKS_H
#define ANTARESXPANSION_ACTIVELINKS_H

#include <Candidate.h>
#include <unordered_map>

class ActiveLink {

public:
	ActiveLink(int idInterco, const std::string& linkName);
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
	void createLinkAndAddCandidate(const CandidateData& candidate_data,
								   const LinkProfile& candidate_profile,
								   const LinkProfile& already_installed_link_profile);
	void launchExceptionIfNoLinkProfileAssociated(const std::string& profileName);
	void launchExceptionIfLinkHasAnotherAlreadyInstalledLinkProfile(const std::string& link_name, const std::string& already_installed_link_profile_name);
	void launchExceptionIfLinkHasAnotherAlreadyInstalledCapacity(const std::string& link, const double& already_installed_link_capacity);
	
	std::unordered_map<std::string, std::string> linkToAlreadyInstalledProfileName;
	std::unordered_map<std::string, double> linkToAlreadyInstalledCapacity;
	const std::vector<CandidateData> _candidateDatas;
	const std::map<std::string, LinkProfile> _profile_map;
    std::vector <ActiveLink> _links;
};

#endif //ANTARESXPANSION_ACTIVELINKS_H
