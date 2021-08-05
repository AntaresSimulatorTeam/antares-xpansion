
#ifndef ANTARESXPANSION_ACTIVELINKS_H
#define ANTARESXPANSION_ACTIVELINKS_H

#include <Candidate.h>
#include <unordered_map>

class ActiveLink {

public:
	ActiveLink() {};
	ActiveLink(int idInterco, const std::string& origin, const std::string& end);
	void setName(const std::string& nameInterco);
	void setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile);

	void addCandidate(const Candidate& candidate);
	bool hasCandidate(const Candidate& candidate) const;

	double direct_profile(size_t i) const;
	double indirect_profile(size_t i) const;
	double already_installed_direct_profile(size_t i) const;
	double already_installed_indirect_profile(size_t i) const;

private:
	int _idInterco;
	std::string _Origin;
	std::string _End;

	std::string _name;
	LinkProfile _profile;
	LinkProfile _already_installed_profile;
	std::string _already_installed_profile_name;

	std::vector<Candidate> _candidates;
};

class ActiveLinks {

public:

    ActiveLinks(){};

    void addCandidate(const CandidateData& data, const LinkProfile& already_install_link_profile, const LinkProfile& link_profile);
	int size() const;

private:
	bool hasCandidate(const Candidate& candidate) const;
    std::unordered_map <int, ActiveLink> _linksHashMap;
};

class ActiveLinksInitializer {

public:
    ActiveLinksInitializer();

    ActiveLinks createActiveLinkFromCandidates(const std::vector<CandidateData>& candidateList,const std::map<std::string, LinkProfile>& profileMap);

    LinkProfile
    getProfile(const std::map<std::string, LinkProfile> &profileMap, const std::string &link_profile_name) const;
};


#endif //ANTARESXPANSION_ACTIVELINKS_H
