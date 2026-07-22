#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <utility>

#include <boost/tokenizer.hpp>

#include "IBendersProblemProvider.h"
#include "BendersProblemFromFile.h"
#include "ConstraintsFileReader.h"
#include "SubproblemWorker.h"
#include "Worker.h"
#include "memoptim_utils.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

enum class CoeffType
{
    constraints = 0,
    objective,
    rhs
};

class FixedSkeletonSubProblemBuilder
{
public:
    FixedSkeletonSubProblemBuilder(const std::filesystem::path& inputRoot,
                                   Logger& logger,
                                   std::string solver_name,
                                   int log_level,
                                   ProblemsFormat format);

    FixedSkeletonSubProblemBuilder(const std::filesystem::path& inputRoot,
                                   Logger& logger,
                                   std::shared_ptr<SolverAbstract> solver);

    std::shared_ptr<SubproblemWorker> create_sub_solver_abstract(std::string sub_name,
                                                                 VariableMap& variable_map,
                                                                 double cut_coefficient_tolerance,
                                                                 double slave_weight);

    void set_added_constraints(std::string sub_name,
                               std::vector<SolverRepresentedRows>& added_constraints);
    int get_sub_number();

private:
    Logger logger_;

    void read_coeffs_and_indices(CoeffType);

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
    /*
    as we build all subproblems on the same solverAbstract object, when we are in warm start case
    we need to keep track of the added constraints, so we add them into the subproblem
    worker when we solve it on the next master iteration
    */
    std::map<std::string, std::vector<SolverRepresentedRows>> added_constraints_per_sub_;
    SolverLogManager solver_log_manager_;
    SolverIO solver_IO_;
    bool micro_iters_;
    bool warm_start_;
};
