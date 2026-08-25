#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>

#include <boost/mpi.hpp>
#include <boost/tokenizer.hpp>

#include "BendersProblemFromFile.h"
#include "IBendersProblemProvider.h"
#include "SkeletonCoefficientSet.h"
#include "SolverRowExtractor.h"
#include "SubproblemBasisCache.h"
#include "SubproblemWorker.h"
#include "Worker.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "skeleton_coefficient_reader.h"

namespace mpi = boost::mpi;

class SubproblemWorkerFactory
{
public:
    SubproblemWorkerFactory(const std::filesystem::path& input_root,
                            Logger& logger,
                            std::string solver_name,
                            int log_level,
                            ProblemsFormat format,
                            std::vector<std::string> sub_problem_names,
                            const SolverLogManager& solver_log_manager);

    SubproblemWorkerFactory(const std::filesystem::path& input_root,
                            Logger& logger,
                            std::shared_ptr<SolverAbstract> solver,
                            std::vector<std::string> sub_problem_names);

    std::shared_ptr<SubproblemWorker> CreateSubSolverAbstract(std::string sub_name,
                                                              VariableMap& variable_map,
                                                              double slave_weight);

    void GetBasis(std::string sub_name);
    void ApplyBasis(const std::string& sub_name);
    int GetSubNumber();
    std::shared_ptr<SolverAbstract> GetSolver();

private:
    Logger logger_;

    void load_coefficient_sets();

    std::filesystem::path input_root_;
    SkeletonCoefficientSet coef_set_;
    SkeletonCoefficientSet obj_set_;
    SkeletonCoefficientSet rhs_set_;
    std::shared_ptr<SolverAbstract> solver_;
    SubproblemBasisCache subproblem_basis_cache_;

    SkeletonCoefficientReader skeleton_coefficient_reader_;
    int SubProblemSolverInitialSize_;
    std::vector<double> skeletonObjCoeffs_ ; 
};
