#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <unordered_map>
#include <utility>

#include <boost/tokenizer.hpp>

#include "BendersProblemFromFile.h"
#include "IBendersProblemProvider.h"
#include "SkeletonCoefficientSet.h"
#include "SolverRowExtractor.h"
#include "SubproblemWorker.h"
#include "Worker.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "memoptim_utils.h"

class SubproblemWorkerFactory
{
public:
    SubproblemWorkerFactory(const std::filesystem::path& input_root,
                            Logger& logger,
                            std::string solver_name,
                            int log_level,
                            ProblemsFormat format,
                            std::vector<std::string> sub_problem_names);

    SubproblemWorkerFactory(const std::filesystem::path& input_root,
                            Logger& logger,
                            std::shared_ptr<SolverAbstract> solver,
                            std::vector<std::string> sub_problem_names);

    std::shared_ptr<SubproblemWorker> CreateSubSolverAbstract(std::string sub_name,
                                                              VariableMap& variable_map,
                                                              double cut_coefficient_tolerance,
                                                              double slave_weight);

    void SetAddedConstraints(std::string sub_name,
                             std::vector<SolverRepresentedRows>& added_constraints);
    int GetSubNumber();

    void SetBasis(std::string sub_name);

private:
    Logger logger_;

    void load_coefficient_sets();

    std::filesystem::path input_root_;
    SkeletonCoefficientSet coef_set_;
    SkeletonCoefficientSet obj_set_;
    SkeletonCoefficientSet rhs_set_;
    std::shared_ptr<SolverAbstract> solver_;
    // at each benders iteration we keep the final solution
    std::unordered_map<std::string, std::pair<std::vector<int>, std::vector<int>>> subpb_basis_;
    /*
    as we build all subproblems on the same solverAbstract object, when we are in warm start case
    we need to keep track of the added constraints, so we add them into the subproblem
    worker when we solve it on the next master iteration
    */
    std::map<std::string, std::vector<SolverRepresentedRows>> added_constraints_per_sub_;
    SolverLogManager solver_log_manager_;
    MemoptimUtils memoptim_utils_;
};
