#pragma once
#include <dlfcn.h>
#include <filesystem>
#include <map>
#include <vector>

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

struct SubProblemIds
{
    const char** subProblems_ids;
    int n_subproblems;
};

struct CandidateLineMasterIterationResult
{
    const char* candidate_line_id;
    int is_invested;
};

struct MasterBendersInput
{
    CandidateLineMasterIterationResult* candidates_res;
    int size;
};

struct FlowN
{
    const char* line_id;
    double value;
};

struct FlowNList
{
    FlowN* flows;
    int size;
};

struct ConstraintsToAdd
{
    const char** constraints;
    int size;
};

using constraintsPerLine = std::map<std::string, std::vector<std::string>>;

using init_julia_FUNC = void (*)(int, char*);
using shut_down_julia_FUNC = void (*)(int);
using jl_load_variables_FUNC = void (*)(SubProblemIds);
using jl_compute_factors_for_microiterations_FUNC = void (*)(MasterBendersInput);
using jl_test_FUNC = void (*)();
using jl_return_constraints_for_micro_iteration_FUNC = ConstraintsToAdd (*)(const char*, FlowNList);
using jl_set_data_path_FUNC = void (*)(const char*);

class Benders_Jl_MICRO_ITERS: public BendersPlugin
{
public:
    Benders_Jl_MICRO_ITERS(const std::filesystem::path& input_root,
                           const CouplingMap& coupling_map);
    virtual ~Benders_Jl_MICRO_ITERS();
    virtual void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                const Logger& logger,
                                const BendersBaseOptions& options,
                                const SolverLogManager& solver_log_manager);
    virtual void OnBendersEnd();
    virtual void OnBendersMasterIterationStart(
      std::map<std::string, double>& benders_invested_master_result);
    virtual void OnBendersMasterIterationEnd();
    virtual void OnBendersMicroIterationStart();
    virtual void OnBendersMicroIterationEnd(std::string sub_name, bool& added_rows);

    void SetSubProblemIDs(const char** subs_ids, int n_subs);

private:
    std::vector<std::string> get_constraints_to_add(ConstraintsToAdd&, std::string);

    void BuildConstraintsReaderMap(const SubproblemsMapPtr& subproblem_map,
                                   const BendersBaseOptions& options,
                                   const SolverLogManager& solver_log_manager);
    bool check_if_constraint_key_is_added(const char* key, std::string sub_name);
    void* handle_;
    std::filesystem::path input_root_;
    std::filesystem::path variables_dictionary_path_;
    SubProblemIds sub_pb_ids_;
    std::map<std::string, std::string> binary_variables_ids_map_;
    std::map<std::string, std::string> variables_to_follow_;
    constraintsPerLine constraints_csv_map_;
    std::vector<std::string> sub_ids_storage_;
    std::vector<const char*> sub_ids_ptrs_;
    std::map<std::string, std::vector<std::string>> added_constraints_per_sub_;
    std::map<std::string, std::string> micro_iterations_config_;
    CouplingMap coupling_map_;
    CouplingMap constraints_coupling_map_;
    SubProblemConstraintMap subproblem_constraint_map_;
    ConstraintsReaderPtrMap constraints_map_;
    Logger _logger;
};
