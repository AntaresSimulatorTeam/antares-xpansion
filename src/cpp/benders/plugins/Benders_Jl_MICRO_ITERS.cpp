#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"
#include "iostream"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <cassert>



Benders_Jl_MICRO_ITERS::Benders_Jl_MICRO_ITERS(const std::filesystem::path& input_root) 
{
    std::cout<<"Benders_Jl_MICRO_ITERS constuctor !!!!!"<<std::endl ; 
    input_root_ = input_root ; 
    std::filesystem::path investment_dictionary_path = input_root / "investment_dictionary.csv" ;
    std::ifstream investment_dict_path (investment_dictionary_path.c_str()) ; 

    if (investment_dict_path.is_open()) 
    {
        std::string row ; 
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(investment_dict_path,row)) 
        {
            Tokenizer tok(row) ; 
            std::vector<std::string> tokens(tok.begin(), tok.end());
            binary_variables_ids_map_[tokens[1]] = tokens[0] ;
        }
    }
    assert(investment_dict_path.is_open()) ; 

    std::filesystem::path variables_dictionary_path = input_root / "variables_dictionary.csv" ; 
    std::ifstream variables_dict(variables_dictionary_path.c_str()) ; 

    if (variables_dict.is_open()) 
    {
        std::string row ; 
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(variables_dict,row)) 
        {
            Tokenizer tok(row) ; 
            std::vector<std::string> tokens(tok.begin(), tok.end());
            variables_to_follow_[tokens[1]] = tokens[0] ;
        }
    }
    
    assert(variables_dict.is_open()) ; 
    
    // std::filesystem::path julia_library_path ="/home/bouchehdahed/studies/0-9_2000//libmylib/lib/libmylib.so" ; 
    std::filesystem::path libmylib_path = input_root_ / "libmylib/lib/libmylib.so" ; 
    handle_ = dlopen(libmylib_path.c_str(),RTLD_NOW) ; 
    if (handle_) 
    {
        init_julia_FUNC init_julia = (init_julia_FUNC) dlsym(handle_,"init_julia") ; 
        init_julia(0,NULL) ;
    }
    assert(handle_) ; 
}

Benders_Jl_MICRO_ITERS::~Benders_Jl_MICRO_ITERS()
{
    std::cout<<"calling Benders_Jl_MICRO_ITERS desructor "<<std::endl ; 
    if (handle_)
    {
        shut_down_julia_FUNC shut_down_julia = (shut_down_julia_FUNC) dlsym(handle_,"shutdown_julia") ; 
        shut_down_julia(0) ; 
        dlclose(handle_) ; 
    }
}

void Benders_Jl_MICRO_ITERS::OnBendersStart() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersStart"<<std::endl ;  
    if (handle_) 
    {
        jl_test_FUNC jl_test = (jl_test_FUNC) dlsym(handle_,"jl_test") ; 
        jl_test() ; 

        int size_subs = sub_pb_ids_.n_subproblems ; 
        for (int i=0; i<size_subs; i++) 
        {
            std::cout<<sub_pb_ids_.subProblems_ids[i]<<std::endl ; 
        }

        jl_load_variables_FUNC jl_load_variables = (jl_load_variables_FUNC) dlsym(handle_,"jl_load_variables") ; 
        jl_load_variables(sub_pb_ids_) ; 
    }
}

void Benders_Jl_MICRO_ITERS::OnBendersEnd()
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersEnd"<<std::endl ; 
}


void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationStart(std::map<std::string,double>& benders_invested_master_result) 
{
    std::cout<<"from Benders_Jl_MICRO_ITERSOnBendersMasterIterationStart"<<std::endl ;  
    CandidateLineMasterIterationResult* candidates_iter_res = new CandidateLineMasterIterationResult[ benders_invested_master_result.size() ] ; 
    int cadidate_pos = 0 ; 
    for (auto& [line,value] :  benders_invested_master_result) 
    {
        std::cout<<"binary map value key "<<line<<" equivalent "<<binary_variables_ids_map_[line]<<std::endl ; 
        auto id_in_csv = binary_variables_ids_map_[line].c_str() ;
        std::cout<<"id_in_csv : "<<id_in_csv<<" value : "<<value<<std::endl ; 
        candidates_iter_res[cadidate_pos] = CandidateLineMasterIterationResult{id_in_csv,value} ; 
        cadidate_pos++;
    }
    
    MasterBendersInput master_benders_input = MasterBendersInput{candidates_iter_res,benders_invested_master_result.size()} ; 
    
    jl_compute_factors_for_microiterations_FUNC compute_factors = (jl_compute_factors_for_microiterations_FUNC) dlsym(handle_,"jl_compute_factors_for_microiterations") ; 
    compute_factors(master_benders_input) ; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationEnd() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMasterIterationEnd"<<std::endl ; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationStart() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMicroIterationStart"<<std::endl; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationEnd() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMicroIterationEnd"<<std::endl ; 
}


void Benders_Jl_MICRO_ITERS::SetSubProblemIDs(const char** subs_ids, int n_subs) 
{

    sub_ids_storage_.clear();
    sub_ids_storage_.reserve(n_subs);
    
    for (int i = 0; i < n_subs; i++) {
        sub_ids_storage_.push_back(subs_ids[i]);
    }
    

    sub_ids_ptrs_.resize(n_subs);
    for (int i = 0; i < n_subs; i++) {
        sub_ids_ptrs_[i] = sub_ids_storage_[i].c_str();
    }
    
    sub_pb_ids_ = SubProblemIds{sub_ids_ptrs_.data(), n_subs};

}



