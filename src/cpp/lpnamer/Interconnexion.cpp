#include "Interconnexion.h"

Interconnexion::Interconnexion(int idInterco, int idOrigin, int idEnd):
    _idInterco(idInterco), _idOrigin(idOrigin), _idEnd(idEnd)
{
}

void Interconnexion::setName(const std::string& nameInterco)
{
    _name = nameInterco;
}

void Interconnexion::setAlreadyInstalledProfile(const LinkProfile& already_installed_profile)
{
    _already_installed_profile = already_installed_profile;
}
