#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

class SubproblemBasisCache
{
public:
    void Store(const std::string& name, SolverAbstract& solver);
    bool TryRestore(const std::string& name, SolverAbstract& solver);

private:
    std::unordered_map<std::string, std::pair<std::vector<int>, std::vector<int>>> basis_per_name_;
};
