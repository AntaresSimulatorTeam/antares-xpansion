#pragma once

#include "Candidate.h"
#include <unordered_map>

class Interconnexions : public std::unordered_map<int, Interconnexion>
{

};

class Interconnexion : public std::vector<Candidate>
{
public:
	Interconnexion(int idInterco, const std::string& origin, const std::string& end);
	void setName(const std::string& nameInterco);
	void setAlreadyInstalledLinkProfile(const LinkProfile& linkProfile);
	void setLinkProfile(const LinkProfile& already_installed_profile);

	void addCandidate(const Candidate& candidate);

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
};

