#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "SkeletonCoefficientSet.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "skeleton_coefficient_reader.h"

class SkeletonConstraintSetLoader
{
public:
    SkeletonConstraintSetLoader(const std::filesystem::path& input_root,
                                Logger& logger,
                                std::string solver_name,
                                int log_level,
                                ProblemsFormat format,
                                std::vector<std::string>&& constraints_names,
                                std::function<void()> fatal_error_handler = {});

    SkeletonConstraintSetLoader(const std::filesystem::path& input_root,
                                Logger& logger,
                                std::shared_ptr<SolverAbstract> solver,
                                std::vector<std::string>&& constraints_names);

    std::shared_ptr<SolverAbstract> LoadConstraintSet(const std::string& constraints_name);

    int GetConstraintsNumber();
    std::shared_ptr<SolverAbstract> GetSolver();

private:
    Logger logger_;

    void read_coeffs_and_indices();

    std::filesystem::path input_root_;

    SkeletonCoefficientSet coef_set_;
    SkeletonCoefficientSet rhs_set_;
    std::shared_ptr<SolverAbstract> solver_;
    SolverLogManager solver_log_manager_;
    SkeletonCoefficientReader skeleton_coefficient_reader_;
    std::function<void()> fatal_error_handler_;
};
