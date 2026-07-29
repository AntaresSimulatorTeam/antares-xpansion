#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

class MemoptimUtils
{
public:
    MemoptimUtils(std::vector<std::string>&& sub_problem_names);

    void read_keyed_coeffs_csv(const std::filesystem::path& csv_path,
                               std::map<std::string, std::vector<double>>& dest);

    void read_indices_csv(const std::filesystem::path& csv_path,
                          std::vector<int>& dest_indices,
                          bool is_col,
                          const std::shared_ptr<SolverAbstract>& solver);

private:
    std::unordered_set<std::string> my_subs_;
};
