#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "BendersProblemFromFile.h"
#include "IBendersProblemProvider.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "memoptim_utils.h"

class FixedSkeletonConstraintsBuilder
{
public:
    FixedSkeletonConstraintsBuilder(const std::filesystem::path& inputRoot,
                                    Logger& logger,
                                    std::string solver_name,
                                    int log_level,
                                    ProblemsFormat format);

    FixedSkeletonConstraintsBuilder(const std::filesystem::path& inputRoot,
                                    Logger& logger,
                                    std::shared_ptr<SolverAbstract> solver);

    std::shared_ptr<SolverAbstract> update_constraints_reader(const std::string& constraints_name);

    int get_constraints_number();

private:
    Logger logger_;

    void read_coeffs_and_indices();

    void build(std::string solver_name,
               const SolverLogManager& solver_log_manager,
               int log_level,
               ProblemsFormat format);

    std::filesystem::path inputRoot_;

    std::map<std::string, std::vector<double>> coeffs_;
    std::vector<int> constraints_col_indices_;
    std::vector<int> constraints_row_indices_;
    std::map<std::string, std::vector<double>> rhs_;
    std::vector<int> rhs_row_indices_;
    std::shared_ptr<SolverAbstract> solver_;
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_;
    SolverLogManager solver_log_manager_;
    SolverIO solver_IO_;
};
