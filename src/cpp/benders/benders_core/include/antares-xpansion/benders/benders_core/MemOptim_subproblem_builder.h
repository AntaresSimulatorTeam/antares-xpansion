#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <utility>

#include <boost/tokenizer.hpp>

#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/ConstraintsFileReader.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/benders_core/Worker.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

enum class CoeffType
{
    constraints = 0,
    objective,
    rhs
};

class MemOptimSubProblemBuilder
{
public:
    MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot,
                              Logger& logger,
                              std::string solver_name,
                              int log_level,
                              ProblemsFormat format);

    MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot,
                              Logger& logger,
                              std::shared_ptr<SolverAbstract> solver);

    std::shared_ptr<SubproblemWorker> create_sub_solver_abstract(std::string sub_name,
                                                                 VariableMap& variable_map,
                                                                 double cut_coefficient_tolerance,
                                                                 double slave_weight);
    int get_sub_number();

private:
    Logger logger_;

    void read_coeffs_and_indices(CoeffType);

    void read_keyed_coeffs_csv(const std::filesystem::path& csv_path,
                               std::map<std::string, std::vector<double>>& dest);
    void read_indices_csv(const std::filesystem::path& csv_path,
                          std::vector<int>& dest_indices,
                          bool is_col);

    // reads the main mps file that will enable creating subproblems
    void build_sub_skeleton(std::string solver_name,
                            const SolverLogManager& solver_log_manager,
                            int log_level,
                            ProblemsFormat format);

    std::filesystem::path inputRoot_;

    std::map<std::string, std::vector<double>> coeffs_;
    std::map<std::string, std::vector<SolverRepresentedRows>> micro_iters_added_rows;
    std::map<std::string, std::vector<double>> obj_coeffs_;
    std::vector<int> rhs_row_indices_;
    std::map<std::string, std::vector<double>> rhs_;
    std::shared_ptr<SolverAbstract> solver_;
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_;
    std::vector<int> constraints_col_indices_;
    std::vector<int> constraints_row_indices_;
    std::vector<int> obj_col_indices_;
    SolverLogManager solver_log_manager_;
    SolverIO solver_IO_;
    bool micro_iters_;
    bool warm_start_;
};
