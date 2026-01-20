#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"

#include "iostream"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <cassert>
#include <string_view>




Benders_Jl_MICRO_ITERS::Benders_Jl_MICRO_ITERS(const std::filesystem::path& input_root) 
{
    // std::cout<<"Benders_Jl_MICRO_ITERS constuctor !!!!!"<<std::endl ; 
    input_root_ = input_root ; 
 
    std::filesystem::path constraints_csv_path = input_root / "constraints_dictionary.csv" ; 
    std::ifstream  constraints_csv_stream(constraints_csv_path.c_str()) ; 

    if (constraints_csv_stream.is_open()) 
    {
        std::string line;
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
            
        while (std::getline(constraints_csv_stream,line)) 
        {
            Tokenizer tok(line) ; 
            std::vector<std::string> tokens(tok.begin(), tok.end());
            std::string key = tokens[0] ;
            std::vector<std::string> values ; 
            if (tokens.size() > 1 ) 
                values.assign(tokens.begin()+1,tokens.end()) ; 
                
            constraints_csv_map_[key] = values ; 
        }
    }


    // std::cout<<"size of constraints_csv_map_ "<<constraints_csv_map_.size()<<std::endl ; 
 
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
            variables_to_follow_[tokens[0]] = tokens[1] ;
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
    // std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersStart"<<std::endl ;  
    if (handle_) 
    {
        jl_test_FUNC jl_test = (jl_test_FUNC) dlsym(handle_,"jl_test") ; 
        jl_test() ; 

        int size_subs = sub_pb_ids_.n_subproblems ; 
        for (int i=0; i<size_subs; i++) 
        {
            // std::cout<<"sub name to set at julia load : " <<sub_pb_ids_.subProblems_ids[i]<<std::endl ; 
            std::string sub_key(sub_pb_ids_.subProblems_ids[i]) ; 
            added_constraints_per_sub_[sub_key] = std::vector<std::string>() ; 
        }

        jl_load_variables_FUNC jl_load_variables = (jl_load_variables_FUNC) dlsym(handle_,"jl_load_variables") ; 
        jl_load_variables(sub_pb_ids_) ; 
    }
}

void Benders_Jl_MICRO_ITERS::OnBendersEnd()
{
    // std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersEnd"<<std::endl ; 
}


void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationStart(std::map<std::string,double>& benders_invested_master_result) 
{
    for (auto& [sub,_] : added_constraints_per_sub_ )
    {
        added_constraints_per_sub_[sub] = std::vector<std::string>() ;  
    }
    
    // std::cout<<"from Benders_Jl_MICRO_ITERSOnBendersMasterIterationStart"<<std::endl ;  
    CandidateLineMasterIterationResult* candidates_iter_res = new CandidateLineMasterIterationResult[ benders_invested_master_result.size() ] ; 
    int cadidate_pos = 0 ; 
    for (auto& [line,value] :  benders_invested_master_result) 
    {
        // std::cout<<"binary map value key "<<line<<" equivalent "<<binary_variables_ids_map_[line]<<std::endl ; 
        auto id_in_csv = binary_variables_ids_map_[line].c_str() ;
        // std::cout<<"id_in_csv : "<<id_in_csv<<" value : "<<value<<std::endl ; 
        candidates_iter_res[cadidate_pos] = CandidateLineMasterIterationResult{id_in_csv,value} ; 
        cadidate_pos++;
    }
    
    MasterBendersInput master_benders_input = MasterBendersInput{candidates_iter_res,benders_invested_master_result.size()} ; 
    
    jl_compute_factors_for_microiterations_FUNC compute_factors = (jl_compute_factors_for_microiterations_FUNC) dlsym(handle_,"jl_compute_factors_for_microiterations") ; 
    compute_factors(master_benders_input) ; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationEnd() 
{
    // std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMasterIterationEnd"<<std::endl ; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationStart() 
{
    // std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMicroIterationStart"<<std::endl; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationEnd(std::shared_ptr<ConstraintsReader> constraint_reader, std::string sub_name, bool & added_rows) 
{
    // std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMicroIterationEnd"<<std::endl ; 


    jl_return_constraints_for_micro_iteration_FUNC jl_return_constraints_for_micro_iteration = (jl_return_constraints_for_micro_iteration_FUNC) dlsym(handle_,"jl_return_constraints_for_micro_iteration") ; 
    auto sub_solution = constraint_reader->get_sub_solution() ; 
    std::vector<FlowN> flows_to_follow ; 
    flows_to_follow.reserve(variables_to_follow_.size()) ; 

    for (auto& [line,line_id] : variables_to_follow_) 
    {
        
        int variable_index = constraint_reader->get_variable_index_in_solution(line_id) ; 
        auto value = sub_solution[variable_index] ; 
        flows_to_follow.push_back(FlowN{line.c_str(),value}) ; 
    }

    FlowNList N_flows = FlowNList{flows_to_follow.data(),flows_to_follow.size()} ; 
    ConstraintsToAdd constraints_to_add =  jl_return_constraints_for_micro_iteration(sub_name.c_str(), N_flows) ; 
    // std::cout<<"size of constraints keys sent from julia "<<constraints_to_add.size<<std::endl ; 
    std::vector<std::string> constraints_to_add_vec = get_constraints_to_add(constraints_to_add,sub_name) ; 
    // std::cout<<"size of constraints_to_add_vec "<<constraints_to_add_vec.size()<<std::endl ; 

    int i(0) ; 
  
    for (auto& constraint_to_add : constraints_to_add_vec) 
    {
            // std::cout<<constraint_to_add<<" "; 
            // std::cout<<"constraint_to_add "<<constraint_to_add<<std::endl; 
            ++i ; 
            constraint_reader->add_rows(constraint_to_add) ; 
    }
    std::cout<<std::endl ; 

    added_rows = constraints_to_add_vec.size() ; 

    // std::cout<<"number of added rows "<<i<<std::endl; 


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


std::vector<std::string> Benders_Jl_MICRO_ITERS::get_constraints_to_add(ConstraintsToAdd& constraints_to_add_obj, std::string sub_name) 
{
    std::vector<std::string> constraints_to_add ; 
    // std::cout<<"constraints to add "; 
    // std::cout<<"to add size "<<constraints_to_add_obj.size<<std::endl; 
    std::cout<<"to add keys : ";
    for (int i=0; i<constraints_to_add_obj.size; i++) 
    {
        if (!check_if_constraint_key_is_added(constraints_to_add_obj.constraints[i],sub_name)) 
        {
            // std::cout<<"constraint to add key "<<constraints_to_add_obj.constraints[i]<<std::endl ; 
            std::string constraint_key(constraints_to_add_obj.constraints[i]) ; 
            std::cout<<constraint_key<<" , "; 
            // std::cout<<constraint_key<<" " ;  
            // std::cout<<"number of constraints to add "<<constraints_csv_map_[constraint_key].size()<<std::endl ; 
            constraints_to_add.insert(constraints_to_add.end(),constraints_csv_map_[constraint_key].begin(),constraints_csv_map_[constraint_key].end() ) ; 
        }
    }
    std::cout<<std::endl ;
    // std::cout<<std::endl ; 
    return constraints_to_add ; 
}



bool Benders_Jl_MICRO_ITERS::check_if_constraint_key_is_added(const char* key, std::string sub_name) 
{

    std::string key_str(key) ; 
    bool found = (std::find(added_constraints_per_sub_[sub_name].begin(), added_constraints_per_sub_[sub_name].end(),key_str) != added_constraints_per_sub_[sub_name].end()) ; 
    if (!found) 
        added_constraints_per_sub_[sub_name].push_back(key_str) ; 
    // std::cout<<key<<" has been already added to "<<sub_name<<std::endl ; 
    return found ;
}
