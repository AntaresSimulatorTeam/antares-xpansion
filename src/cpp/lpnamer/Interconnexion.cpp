#include "Interconnexion.h"

Interconnexion::Interconnexion(int idInterco, const std::string& origin, const std::string& end):
    _idInterco(idInterco), _Origin(origin), _End(end)
{
}

void Interconnexion::setName(const std::string& nameInterco)
{
    _name = nameInterco;
}

void Interconnexion::setLinkProfile(const LinkProfile& linkProfile)
{
    _profile = linkProfile;
}

void Interconnexion::addCandidate(const Candidate& candidate)
{
    if (_profile.empty() && candidate.has_link_profile())
    {
        _profile = candidate._profile;
    }
    if (_already_installed_profile.empty() && candidate.has_already_installed_link_profile())
    {
        _already_installed_profile = candidate._already_installed_profile;
    }
    this->push_back(candidate);
}

double Interconnexion::direct_profile(size_t i) const
{
    return _profile.getDirectProfile(i);
}

double Interconnexion::indirect_profile(size_t i) const
{
    return _profile.getIndirectProfile(i);
}

double Interconnexion::already_installed_direct_profile(size_t i) const
{
    return _already_installed_profile.getDirectProfile(i);
}

double Interconnexion::already_installed_indirect_profile(size_t i) const
{
    return _already_installed_profile.getIndirectProfile(i);
}

void Interconnexion::setAlreadyInstalledLinkProfile(const LinkProfile& already_installed_profile)
{
    _already_installed_profile = already_installed_profile;
}
