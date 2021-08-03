#pragma once

#include "Candidate.h"

class Interconnexion : public std::vector<Candidate>
{
	Interconnexion(int idInterco, int idOrigin, int idEnd);
	void setName(const std::string& nameInterco);
	void setAlreadyInstalledProfile(const LinkProfile& already_installed_profile);

private:
	int _idInterco;
	int _idOrigin;
	int _idEnd;

	std::string _name;
	LinkProfile _already_installed_profile;
};

