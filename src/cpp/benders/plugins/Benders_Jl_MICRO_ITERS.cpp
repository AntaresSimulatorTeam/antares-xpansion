#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"
#include "iostream"
#include <fstream>
#include <boost/tokenizer.hpp>



Benders_Jl_MICRO_ITERS::Benders_Jl_MICRO_ITERS(std::filesystem::path jl_lib_path) 
{
    std::cout<<"Benders_Jl_MICRO_ITERS constuctor !!!!!"<<std::endl ; 
    std::ifstream investment_dict_path ("./investment_dictionary.csv") ; 

    if (investment_dict_path.is_open()) 
    {
        std::cout<<"investment_dictionnary.csv is opened correctly !!!"<<std::endl ; 
        std::string row ; 
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(investment_dict_path,row)) 
        {
            Tokenizer tok(row) ; 
            std::vector<std::string> tokens(tok.begin(), tok.end());
            binary_variables_ids_map_[tokens[1]] = tokens[0] ;
        }
    }

    handle_ = dlopen(jl_lib_path.c_str(),RTLD_NOW) ; 
    if (handle_) 
    {
        std::cout<<"handle_ is opened correclty for Benders_Jl_MICRO_ITERS !!!"<<std::endl ; 
        init_julia_FUNC init_julia = (init_julia_FUNC) dlsym(handle_,"init_julia") ; 
        init_julia(0,NULL) ;
    }
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
        char* id_in_csv = new char[binary_variables_ids_map_[line].size() + 1];
        std::strcpy(id_in_csv, binary_variables_ids_map_[line].c_str());
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


void Benders_Jl_MICRO_ITERS::SetSubProblemIDs(char** subs_ids, int n_subs) 
{
    sub_pb_ids_ = SubProblemIds{subs_ids,n_subs} ; 
}



