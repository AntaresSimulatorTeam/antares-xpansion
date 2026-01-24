/*
This header file contains the necessary classes and structure to implement the benders plugin mechanism for the micro iteration.
As some of necessary code components for micro iterations is written in Julia, this code will be compiled into a dynamic library,
loaded into Benders_Jl_MICRO_ITERS class.
*/

#pragma once
#include <dlfcn.h>
#include <filesystem>
#include <map>
#include <vector>
#include <chrono> 
#include <memory>

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "antares-xpansion/benders/logger/MicroIterationsLog.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"



/*
    This structure will be used to set the subproblems ids on the Julia side
    @members :
        - subProblems_ids : a pointer to an array of c-style string
        - n_subproblems : number of subproblems
*/
struct SubProblemIds
{
    const char** subProblems_ids;
    int n_subproblems;
};


/*
    This structure will be used after solving the master problem. 
    @members : 
        - candidate_line_id : id of the candidate line 
        - is_invested : 0 if not invest, 1 if invested

*/
struct CandidateLineMasterIterationResult
{
    const char* candidate_line_id;
    int is_invested;
};

/*
    This structure is an array of the list above
    @members : 
        - candidates_res : a pointer to an array of CandidateLineMasterIterationResult 
        - size : number of candidate lines 
*/
struct MasterBendersInput
{
    CandidateLineMasterIterationResult* candidates_res;
    int size;
};


/*
    This structure will be used in the Julia code to compute the microiteration violated constraints 
    @members : 
        - flow_id : the id of the flow we need to evaluate the constraints. 
        - value : its value after solving the subproblem
*/
struct FlowN
{
    const char* flow_id;
    double value;
};

/*
    This structure is an array of the list above 
    @members : 
        - flows : a pointer to an array of FlowN
        - size : number of flows we need 
*/
struct FlowNList
{
    FlowN* flows;
    int size;
};

/*
    This structure is what will be rendered by the Julia code at each micro iteration. 
    @members : 
        - constraints : a pointer to an array of c-style strings of keys of constraints to add to the subproblem 
        - size : number of constraints to add 
*/
struct ConstraintsToAdd
{
    const char** constraints;
    int size;
};


/*
    This type will be the map resulting of reading co,nstraints_dictionary.csv. 
    This file have the following structure :  
    constraints_key_1 : constraint_to_add_1, ...., constraints_to_add_n
                            .
                            .
    constraints_key_l : constraint_to_add_1, ..., constraints_to_add_k
*/
using constraintsPerLine = std::map<std::string, std::vector<std::string>>;


/*
    This type will be used for the Julia function init_julia. 
    It is necessary to be able to use the Julia functions we need for
    microiterations workflow 
    Inputs should be set to (0,NULL)
*/
using init_julia_FUNC = void (*)(int, char*);

/*
    This type will be used to end the Julia process in the benders code. 
    Input shoule be (0)
*/
using shut_down_julia_FUNC = void (*)(int);

/*
    This type will be used for the Julia function jl_load_variables. 
    That function is used to read the necessary .jls and .csv file on the julia side. 
    It takes as an input an object of type SubProblemIds

*/
using jl_load_variables_FUNC = void (*)(SubProblemIds);

/*
    This type will be used for the julia function jl_compute_factors_for_microiterations. 
    That function is used to compute the new PTDF at each master iteration. 
    It takes as an input an object of type MasterBendersInput that contains the result 
    of solving the master problem 
*/
using jl_compute_factors_for_microiterations_FUNC = const char* (*)(MasterBendersInput, int);

/*
    This type will be used for the Julia function jl_return_constraints_for_micro_iteration.
    That function is called at each microiteration and for each subproblem to get the list of constraints to add. 
    It takes as inputs : 
        - a c-style string : the id of the subproblem 
        - FlowNList object : the list of flows we need to compute violated constraints
*/
using jl_return_constraints_for_micro_iteration_FUNC = ConstraintsToAdd (*)(const char*, FlowNList);


/*
    Implementation of BendersPlugin to manage the microiterations workflow
*/

class Benders_Jl_MICRO_ITERS: public BendersPlugin
{
public:

    /*
        Constructor 
        Inputs : 
            - options : study options
            - coupling_map : coupling map (master and sub to variables)
    */
    Benders_Jl_MICRO_ITERS(const SimulationOptions& options, 
                           const CouplingMap& coupling_map);

    /*
        Default destrucor
    */
    virtual ~Benders_Jl_MICRO_ITERS();

    /*
        Implementation of benders start call back
    */
    virtual void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                const Logger& logger,
                                const BendersBaseOptions& options,
                                const SolverLogManager& solver_log_manager);
    
    
    /*
        Implementation of benders end call back
    */                           
    virtual void OnBendersEnd();

    /*
        Implementation of master iteration start call back  
    */
    virtual void OnBendersMasterIterationStart(
      std::map<std::string, double>& master_out, int& num_iter);
    
    /*
        Implementation of master iteration end call back     
    */
    virtual void OnBendersMasterIterationEnd();
    /*
        Implementation of micro iteration start call back 
    */
    virtual void OnBendersMicroIterationStart();

    /*
        Implementation of micro iteration end callback
    */
    virtual void OnBendersMicroIterationEnd(std::string sub_name, bool& added_rows,std::string solving_time);
    
    /*
        This functions sets sub_pb_ids_ which is necessary in handeling the julia code
        @inputs 
            - subs_ids : a pointer to an array of c-style string which are the subproblem ids 
            - n_subs : number of sub problem
    */
    void SetSubProblemIDs(const char** subs_ids, int n_subs);

private:
    /*
        This function is used to get the constraints to add to the subproblem from 
        the list of constraint key computed after solving the subproblem
        @inputs : 
            - constraints_to_add : list of the constraints key returned by the Julia code
            - sub_name : name of the subproblem
    */
    std::vector<std::string> get_constraints_to_add(ConstraintsToAdd& constraints_to_add, std::string sub_name);
    

    /*
        This function is used to build the ConstraintsReader objects associated to each subproblem
        @inputs : 
            - subproblem_map : the map to the subproblem workers 
            - options : study options
            - solver_log_manager : solver log manger 
    */
    void BuildConstraintsReaderMap(const SubproblemsMapPtr& subproblem_map,
                                   const BendersBaseOptions& options,
                                   const SolverLogManager& solver_log_manager);

    /*
        This function is used to check if a constraint key rendered by the julia cde 
        has been added or not to the subproblem worker. 
        @inputs 
            - key : constraint key to check 
            - sub_name : subproblem name 
    */
    bool check_if_constraint_key_is_added(const char* key, std::string sub_name);
    void* handle_;
    const SimulationOptions& options_ ; 
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
    std::shared_ptr<MicroIterationsLog> micro_iterations_logger_; 
    bool warm_start_; 
};
