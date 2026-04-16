/*
This header file contains the necessary classes and structure to implement the benders plugin
mechanism for the micro iteration. As some of necessary code components for micro iterations is
written in Julia, this code will be compiled into a dynamic library, loaded into
Benders_Jl_MICRO_ITERS class.
*/

#pragma once
#include <chrono>
#include <dlfcn.h>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/logger/MicroIterationsLog.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

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
struct CandidateLineInvestmentStatus
{
    const char* candidate_line_id;
    int is_invested;
};

/*
    This structure is an array of the list above
    @members :
        - candidates_res : a pointer to an array of CandidateLineInvestmentStatus
        - size : number of candidate lines
*/
struct CandidateLineInvestmentStatusList
{
    CandidateLineInvestmentStatus* candidates_res;
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
        - constraints : a pointer to an array of c-style strings of keys of constraints to add to
   the subproblem
        - size : number of constraints to add
*/
struct ViolatedFlowConstraints
{
    const char** constraints;
    int size;
};

/*
    We will compute the necessary factors at each master iteration at one proc
    Then we will serialize them and set them on the other proc from c++
    This struct contain pointer to these serialized objects
    @memebers
        - HVDC_dict_serialized : serialized HVDC dict
        - dict_incident_factors_serialized : dict incidenet factors serialized
        - all_monitored_branches_serialized : serialized monitored branches
*/

struct SerializedObject
{
    uint8_t* bytes_ptr;
    int bytes_length;
};

struct SerializedFactors
{
    SerializedObject HVDC_dict_serialized;
    SerializedObject dict_incident_factors_serialized;
    SerializedObject all_monitored_branches_serialized;
};

struct SerializedBuffers
{
    std::vector<uint8_t> HVDC_dict_serialized_buff;
    std::vector<uint8_t> dict_incident_factors_serialized_buff;
    std::vector<uint8_t> all_monitored_branches_serialized_buff;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & HVDC_dict_serialized_buff;
        ar & dict_incident_factors_serialized_buff;
        ar & all_monitored_branches_serialized_buff;
    }
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
using jl_load_variables_FUNC = void (*)(SubProblemIds, int);

/*
    This type will be used for the julia function jl_compute_factors_for_microiterations.
    That function is used to compute the new PTDF at each master iteration.
    It takes as an input an object of type MasterBendersInput that contains the result
    of solving the master problem
*/
using jl_compute_factors_for_microiterations_FUNC = SerializedFactors (*)(
  CandidateLineInvestmentStatusList,
  int);

/*
    This type will be used for the Julia function jl_return_constraints_for_micro_iteration.
    That function is called at each microiteration and for each subproblem to get the list of
   constraints to add. It takes as inputs :
        - a c-style string : the id of the subproblem
        - FlowNList object : the list of flows we need to compute violated constraints
        - SerializedFactors object : contains the serailizd factors computed at the master iteration
   on proc 0
*/
using jl_return_constraints_for_micro_iteration_FUNC = ViolatedFlowConstraints (*)(const char*,
                                                                                   FlowNList);

using jl_call_GC_FUNC = void (*)();

/*
    Since the input julia that allow updating the factos at each master iterations are quite heavy.
    we can't compute the new ptdf and the different factors needed at benders master iteration at
   each process. The idea is to do the computing on the proc 0 on julia side, serialize these
   object, send a pointer to the c++ abd set them on the other procs so we can update compute
   violated constraints at each proc
*/

using jl_deserialize_factors_FUNC = void (*)(SerializedFactors);

using jl_clean_buffers_FUNC = void (*)();

using on_Benders_start_Func = void (*)(SubProblemIds, int,std::filesystem::path,
                std::filesystem::path , 
                  bool ,
                  mpi::communicator* ,
                  int );
using on_Benders_end_Func = void (*)();
using on_Benders_iteration_start = void (*)();
using on_Benders_iteration_end = void (*)();
using on_Benders_master_resolution_start = void (*)(
  std::map<std::string, double>&,
  int&,
  mpi::communicator*,
  std::map<std::string, std::vector<std::string>>&,
  std::filesystem::path input_root);

using on_Benders_master_resolution_end = void (*)();
using on_Benders_micro_iteration_start = void (*)();
using on_Benders_micro_iteration_end = void (*)(std::string sub_name,
                                                bool& added_rows,
                                                std::string solving_time,
                                                std::vector<double> sub_solution,
                                                std::vector<int> variables_indices_vector,
                                                std::vector<std::string>& variables_names_vector,
                                                std::filesystem::path input_root, 
                                                std::vector<std::string>& contraints_to_add_vec, 
                                                int, 
                                                int);
using on_Benders_sub_resolution_start = void (*)();
using on_Benders_sub_resolution_end = void (*)(std::string sub_name, int num_micro_iter);

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
                           const CouplingMap& coupling_map,
                           mpi::communicator* world);

    /*
        Default destrucor
    */
    virtual ~Benders_Jl_MICRO_ITERS() = default;

    /*
        Implementation of benders start call back
    */
    virtual void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                const Logger& logger,
                                const BendersBaseOptions& options,
                                const SolverLogManager& solver_log_manager);

    void OnBendersIterationStart();
    void OnBendersIterationEnd();

    /*
        Implementation of benders end call back
    */
    virtual void OnBendersEnd();

    /*
        Implementation of master iteration start call back
    */
    virtual void OnBendersMasterResolutionStart(std::map<std::string, double>& master_out,
                                                int& num_iter);

    /*
        Implementation of master iteration end call back
    */
    virtual void OnBendersMasterResolutionEnd();
    /*
        Implementation of micro iteration start call back
    */
    virtual void OnBendersMicroIterationStart();

    /*
        Implementation of micro iteration end callback
    */
    virtual void OnBendersMicroIterationEnd(std::string sub_name,
                                            bool& added_rows,
                                            std::string solving_time,
                                            int num_master_iter, 
                                            int num_micro_iter);

    virtual void OnBendersSubResolutionStart();
    virtual void OnBendersSubResolutionEnd(std::string sub_name, int num_micro_iter);

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
    // */
    // std::vector<std::string> get_constraints_to_add(ViolatedFlowConstraints& constraints_to_add,
    //                                                 std::string sub_name);

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
    // bool check_if_constraint_key_is_added(const char* key, std::string sub_name);

    // BuildSubProblemConstaintMap()
    void read_micro_iteration_config_file();
    // void read_constraints_dict();
    // void read_investment_dictionnary();
    // void read_variables_dictionnary();
    void read_variables_to_follow_ids();
    void read_variable_names();
    void build_variables_to_follow_indices_vector(std::string sub_name);
    const std::map<std::string, std::vector<int>>& get_variables_to_follow_indeices_vector() ; 

    mpi::communicator* _world;
    void* handle_;

    void* handle_2 ; 
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
    std::map<std::string,bool> is_variable_names_indices_created_ ; 
    std::map<std::string,std::vector<int>> variables_to_follow_indices_per_sub_ ; 
    const SimulationOptions& options_;
    std::filesystem::path input_root_;
    std::filesystem::path variables_dictionary_path_;
    SubProblemIds sub_pb_ids_;
    std::map<std::string, std::string> binary_variables_ids_map_;
    constraintsPerLine constraints_csv_map_;
    std::vector<std::string> sub_ids_storage_;
    std::vector<const char*> sub_ids_ptrs_;
    std::map<std::string, std::vector<std::string>> added_constraints_per_sub_;
    std::map<std::string, std::string> micro_iterations_config_;
    std::vector<std::string> variables_to_follow_;
    CouplingMap coupling_map_;
    CouplingMap constraints_coupling_map_;
    SubProblemConstraintMap subproblem_constraint_map_;
    ConstraintsReaderPtrMap constraints_map_;
    Logger _logger;
    bool warm_start_;
};
