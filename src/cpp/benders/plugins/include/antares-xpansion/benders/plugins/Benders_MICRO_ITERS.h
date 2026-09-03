/*
This header file contains the necessary classes and structure to implement the benders plugin
mechanism for the micro iteration. As some of necessary code components for micro iterations is
written in Julia, this code will be compiled into a dynamic library, loaded into
Benders_MICRO_ITERS class.
*/

#pragma once
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_core/SkeletonConstraintSetLoader.h"
#include "antares-xpansion/benders/benders_core/SubproblemConstraintsManager.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/benders_core/IBendersPlugin.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

using on_Benders_start_Func = void (*)(std::filesystem::path,
                                       std::filesystem::path,
                                       bool,
                                       mpi::communicator*,
                                       int);
using on_Benders_end_Func = void (*)();
using on_Benders_iteration_start = void (*)();
using on_Benders_iteration_end = void (*)();
using on_Benders_master_resolution_start = void (*)();
using on_Benders_master_resolution_end = void (*)(std::map<std::string, double>&,
                                                  int&,
                                                  mpi::communicator*,
                                                  std::filesystem::path input_root);

using on_Benders_micro_iteration_start = void (*)();
using on_Benders_micro_iteration_end = void (*)(std::string sub_name,
                                                std::string solving_time,
                                                std::vector<double> sub_solution,
                                                std::vector<int>& variables_indices_vector,
                                                std::vector<std::string>& variables_names_vector,
                                                std::filesystem::path input_root,
                                                std::vector<std::string>& contraints_to_add_vec,
                                                int,
                                                int);
using on_Benders_sub_resolution_start = void (*)();
using on_Benders_sub_resolution_end = void (*)();

/*
    Implementation of BendersPlugin to manage the microiterations workflow
*/

class Benders_MICRO_ITERS final: public IBendersPlugin
{
public:
    /*
        Constructor
        Inputs :
            - options : study options
            - coupling_map : coupling map (master and sub to variables)
    */
    Benders_MICRO_ITERS(const SimulationOptions& options,
                        const CouplingMap& coupling_map,
                        mpi::communicator* world);

    /*
        Default destrucor
    */
    virtual ~Benders_MICRO_ITERS() = default;

    /*
        Implementation of benders start call back
    */
    void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                        const Logger& logger,
                        const BendersBaseOptions& options,
                        const SolverLogManager& solver_log_manager,
                        std::shared_ptr<SolverAbstract> sub_problem_solver) override;

    void OnBendersIterationStart() override;
    void OnBendersIterationEnd() override;

    /*
        Implementation of benders end call back
    */
    void OnBendersEnd() override;

    /*
        Implementation of master resolution end call back
    */
    void OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                      int& num_iter) override;

    /*
        Implementation of master resolution start call back
    */
    void OnBendersMasterResolutionStart() override;

    /*
        Implementation of micro iteration start call back
    */
    void OnBendersMicroIterationStart() override;

    /*
        Implementation of micro iteration end callback
    */
    void OnBendersMicroIterationEnd(std::string sub_name,
                                    bool& added_rows,
                                    std::string solving_time,
                                    int num_master_iter,
                                    int num_micro_iter) override;

    void OnBendersSubResolutionStart(const std::shared_ptr<SubproblemWorker>& sub_worker,
                                     std::string sub_name) override;
    void OnBendersSubResolutionEnd() override;

    bool ShouldRestoreSubproblemBasis() const override;

    /*
        This functions sets sub_pb_ids_ which is necessary in handeling the julia code
        @inputs
            - subs_ids : a pointer to an array of c-style string which are the subproblem ids
            - n_subs : number of sub problem
    */
    void SetSubProblemIDs(std::vector<std::string>&);

private:
    /*
        This function is used to get the constraints to add to the subproblem from
        the list of constraint key computed after solving the subproblem
        @inputs :
            - constraints_to_add : list of the constraints key returned by the Julia code
            - sub_name : name of the subproblem
    // */
    // std::vector<std::string> get_constraints_to_add(ViolatedFlowConstraints& constraints_to_add,
    //                                                 std::string sub_name);

    /*
        This function is used to build the SubproblemConstraintsManager objects associated to each
       subproblem
        @inputs :
            - subproblem_map : the map to the subproblem workers
            - options : study options
            - solver_log_manager : solver log manger
    */
    void build_subproblem_constraints_manager_map(const SubproblemsMapPtr& subproblem_map,
                                                  const BendersBaseOptions& options,
                                                  const SolverLogManager& solver_log_manager);

    void build_skeleton_constraint_set_loader(const BendersBaseOptions& options);

    void read_micro_iteration_config_file();

    void read_variable_names_to_follow();
    void build_variables_to_follow_indices_vector();

    /*
        Returns the SubproblemConstraintsManager currently representing sub_name's added
        constraints, regardless of which CACHE_PROBLEMS storage strategy is in use.
    */
    SubproblemConstraintsManagerPtr GetActiveManager(const std::string& sub_name);

    /*
        Records constraint as added for sub_name so it can be replayed on a future Benders
        iteration. A no-op unless warm_start_ is set (CACHE_PROBLEMS==0 never needs tracking,
        since its manager is never rebuilt/reset).
    */
    void TrackAddedConstraint(const std::string& sub_name, const std::string& constraint);

    /*
        Re-applies the constraints tracked for sub_name onto its currently active manager.
        A no-op unless warm_start_ is set.
    */
    void ReplayPersistedConstraints(const std::string& sub_name);

    /*
        Clears the tracked added-constraints list for every subproblem, so nothing gets
        replayed on the next Benders iteration.
    */
    void ClearPersistedConstraintsTracking();

    /*
        CACHE_PROBLEMS>=2 only: strips sub_problem_solver_ back down to
        InitialSubProblemSolverSize_ rows, removing whatever rows were added for
        whichever subproblem last used this shared skeleton solver on this rank.
        A no-op if it is already at that size.
    */
    void ResetSharedSolverToBase();

    mpi::communicator* _world;
#ifdef _WIN32
    HMODULE handle_;
#else
    void* handle_;
#endif

    on_Benders_start_Func onBendersStartPlugin_;
    on_Benders_end_Func OnBendersEndPlugin_;
    on_Benders_iteration_start OnBendersIterationStart_;
    on_Benders_iteration_end OnBendersIterationEnd_;
    on_Benders_master_resolution_start OnBendersMasterResolutionStart_;
    on_Benders_master_resolution_end OnBendersMasterResolutionEnd_;
    on_Benders_micro_iteration_start OnBendersMicroIterationStart_;
    on_Benders_micro_iteration_end OnBendersMicroIterationEnd_;
    on_Benders_sub_resolution_start OnBendersSubResolutionStart_;
    on_Benders_sub_resolution_end OnBendersSubResolutionEnd_;
    std::map<std::string, std::vector<int>> variables_to_follow_indices_per_sub_;
    const SimulationOptions& options_;

    std::vector<std::string> sub_names_;
    std::map<std::string, std::vector<std::string>> added_constraints_per_sub_;
    std::map<std::string, std::string> micro_iterations_config_;
    std::vector<std::string> variables_to_follow_;
    CouplingMap coupling_map_;
    CouplingMap constraints_coupling_map_;
    SubProblemConstraintMap subproblem_constraint_map_;
    SubproblemConstraintsManagerPtrMap constraints_map_;
    const SolverLogManager* solver_log_manager_ = nullptr;
    Logger _logger;
    bool warm_start_;
    std::shared_ptr<SkeletonConstraintSetLoader> constraint_set_loader_;
    SubproblemConstraintsManagerPtr subproblem_constraints_manager_;
    std::shared_ptr<SolverAbstract> sub_problem_solver_;
    int InitialSubProblemSolverSize_;
};
