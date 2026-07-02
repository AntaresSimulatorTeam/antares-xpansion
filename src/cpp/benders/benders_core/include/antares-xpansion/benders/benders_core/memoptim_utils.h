#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

namespace memoptim_utils
{

void read_keyed_coeffs_csv(const std::filesystem::path& csv_path,
                           std::map<std::string, std::vector<double>>& dest);

void read_indices_csv(const std::filesystem::path& csv_path,
                      std::vector<int>& dest_indices,
                      bool is_col,
                      const std::shared_ptr<SolverAbstract>& solver);

} // namespace memoptim_utils
