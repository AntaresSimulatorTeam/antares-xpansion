#pragma once 
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include <filesystem>
#include <dlfcn.h>
#include <map>
#include <vector>


struct SubProblemIds 
{
    const char** subProblems_ids ; 
    int n_subproblems ; 
} ; 


struct CandidateLineMasterIterationResult 
{
    const char* candidate_line_id ; 
    int is_invested ; 
}; 


struct MasterBendersInput
{
    CandidateLineMasterIterationResult* candidates_res; 
    int size; 
} ; 

using init_julia_FUNC = void (*) (int, char*); 
using shut_down_julia_FUNC = void(*)(int) ; 
using jl_load_variables_FUNC = void(*) (SubProblemIds) ; 
using jl_compute_factors_for_microiterations_FUNC = void(*)(MasterBendersInput) ; 
using jl_test_FUNC = void(*)() ; 



class Benders_Jl_MICRO_ITERS : public BendersPlugin 
{
    public : 
        Benders_Jl_MICRO_ITERS(const std::filesystem::path& input_root) ; 
        virtual ~Benders_Jl_MICRO_ITERS()  ;
        virtual void OnBendersStart()  ; 
        virtual void OnBendersEnd()  ;  
        virtual void OnBendersMasterIterationStart(std::map<std::string,double>& benders_invested_master_result)  ;  
        virtual void OnBendersMasterIterationEnd()  ;  
        virtual void OnBendersMicroIterationStart()  ; 
        virtual void OnBendersMicroIterationEnd(std::shared_ptr<ConstraintsReader> constraint_reader) ; 

        void SetSubProblemIDs(const char** subs_ids, int n_subs) ; 

    private : 
        void* handle_ ; 
        std::filesystem::path input_root_ ; 
        SubProblemIds sub_pb_ids_ ; 
        std::map<std::string,std::string> binary_variables_ids_map_ ; 
        std::map<std::string,std::string> variables_to_follow_ ; 
        std::vector<std::string> sub_ids_storage_;  
        std::vector<const char*> sub_ids_ptrs_;  
    
} ; 