#include "antares-xpansion/multisolver_interface/SolverConfig.h"

#include <algorithm>
#include <utility>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

SolverConfig::SolverConfig(std::string solver_name)
{
    init(std::move(solver_name));
}

bool SolverConfig::operator==(const std::string& rhs) const
{
    return std::ranges::equal(name,
                              rhs,
                              [](char a, char b) { return ::tolower(a) == ::tolower(b); });
}

SolverConfig& SolverConfig::operator=(const std::string& rhs)
{
    init(rhs);
    return *this;
}

void SolverConfig::init(std::string solver_name)
{
    // Locally defined constant to prevent static initialisation error
    const std::map<std::string, bool> save_restore_support = {{"clp", false},
                                                              {"cbc", false},
                                                              {"coin", false},
                                                              {"xpress", true}};
    name = std::move(solver_name);
    std::ranges::transform(name, name.begin(), ::tolower);
    if (save_restore_support.find(name) == save_restore_support.end())
    {
        throw LogUtils::XpansionError<std::invalid_argument>("Invalid solver name", LOGLOCATION);
    }
    save_restore_supported = save_restore_support.at(name);
    use_save_restore = save_restore_supported;
}

std::filesystem::path SolverConfig::FileName(const std::string& problemName)
{
    const std::string save_ext = ".svf";
    const std::string default_ext = ".mps";
    std::filesystem::path path{problemName};
    if (use_save_restore)
    {
        path.replace_extension(save_ext);
    }
    else
    {
        path.replace_extension(default_ext);
    }
    return path;
}
