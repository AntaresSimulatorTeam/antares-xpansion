#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/mpi.hpp>
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "skeleton_coefficient_reader.h"

class SkeletonCoefficientSet
{
public:
    SkeletonCoefficientSet() = default;

    void Load(SkeletonCoefficientReader& skeleton_coefficient_reader,
              const std::filesystem::path& coeffs_csv,
              std::optional<std::filesystem::path> col_indices_csv,
              std::optional<std::filesystem::path> row_indices_csv,
              const std::shared_ptr<SolverAbstract>& solver,
              Logger& logger, 
              boost::mpi::communicator* world );

    std::vector<double>& CoefficientsFor(const std::string& name);
    std::vector<int>& RowIndices();
    std::vector<int>& ColIndices();
    int Count() const;

private:
    std::map<std::string, std::vector<double>> coeffs_;
    std::vector<int> row_indices_;
    std::vector<int> col_indices_;
    boost::mpi::communicator* world_ = nullptr;
    Logger logger_ ; 
};
