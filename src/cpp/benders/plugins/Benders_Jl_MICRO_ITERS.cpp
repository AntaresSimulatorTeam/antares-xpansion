// Copyright (C) 2026 Hedi Bouchehda.
/*
    Implementation of Benders_Jl_MICRO_ITERS
*/

#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string_view>
#include <boost/tokenizer.hpp>
#include <chrono>

#include "iostream"

Benders_Jl_MICRO_ITERS::Benders_Jl_MICRO_ITERS(const SimulationOptions& options,
                                               const CouplingMap& coupling_map)
                                               : options_(options)
{
    coupling_map_ = coupling_map;

    CouplingMapGenerator::BuildSubProblemConstaintMap(coupling_map_,
                                                      subproblem_constraint_map_,
                                                      constraints_coupling_map_);

    input_root_ = options_.INPUTROOT ;
    warm_start_ = true ; 

    //Reading the micro iterations configuration file
    std::filesystem::path mirco_iterations_options_path = input_root_
                                                          / "micro_iterations_config.txt";
    std::ifstream micro_iterations_options_stream(mirco_iterations_options_path.c_str());
    

    if (micro_iterations_options_stream.is_open())
    {
        std::string line;
        while (std::getline(micro_iterations_options_stream, line))
        {
            std::istringstream iss(line);
            std::string key, value;

            if (std::getline(iss, key, '=') && std::getline(iss, value))
            {
                if (key == "warm_start")
                {
                    if (value == "0") 
                        warm_start_ = false ; 
                }
                else 
                    micro_iterations_config_[key] = value;
                
            }
        }
    }
    else
    {
        std::cerr<<"unable to open : "<<mirco_iterations_options_path.c_str()<<std::endl ; 
        exit(EXIT_FAILURE) ; 
    }

    //Reading constraints dictionary 
    std::filesystem::path constraints_csv_path = input_root_ / "constraints_dictionary.csv";
    std::ifstream constraints_csv_stream(constraints_csv_path.c_str());

    if (constraints_csv_stream.is_open())
    {
        std::string line;
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(constraints_csv_stream, line))
        {
            Tokenizer tok(line);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            std::string key = tokens[0];
            std::vector<std::string> values;
            if (tokens.size() > 1)
            {
                values.assign(tokens.begin() + 1, tokens.end());
            }

            constraints_csv_map_[key] = values;
        }
    }
    else 
    {
        std::cerr<<"unable to open : "<<constraints_csv_path.c_str()<<std::endl; 
        exit(EXIT_FAILURE) ; 
    }


    //Reading investement dictionary 
    std::filesystem::path investment_dictionary_path = input_root_ / "investment_dictionary.csv";
    std::ifstream investment_dict_path(investment_dictionary_path.c_str());

    if (investment_dict_path.is_open())
    {
        std::string row;
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(investment_dict_path, row))
        {
            Tokenizer tok(row);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            binary_variables_ids_map_[tokens[1]] = tokens[0];
        }
    }
    else 
    {
        std::cerr<<"unable to open : "<<investment_dictionary_path.c_str()<<std::endl; 
        exit(EXIT_FAILURE) ; 
    }


    //Reading variables dictionary 
    variables_dictionary_path_ = input_root_ / "variables_dictionary.csv";
    std::ifstream variables_dict(variables_dictionary_path_.c_str());

    if (variables_dict.is_open())
    {
        std::string row;
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(variables_dict, row))
        {
            Tokenizer tok(row);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            variables_to_follow_[tokens[0]] = tokens[1];
        }
    }
    else 
    {
        std::cerr<<"unable to open : "<<variables_dictionary_path_.c_str()<<std::endl; 
        exit(EXIT_FAILURE) ; 
    }

    std::filesystem::path libmylib_path = micro_iterations_config_["jl_library_path"];
    handle_ = dlopen(libmylib_path.c_str(), RTLD_NOW);
    if (handle_)
    {
        init_julia_FUNC init_julia = (init_julia_FUNC)dlsym(handle_, "init_julia");
        init_julia(0, NULL);
    }
    else 
    {
        std::cerr<<"unable to open : "<<libmylib_path.c_str()<<std::endl; 
        exit(EXIT_FAILURE) ; 
    }

    if (options_.LOG_LEVEL >= 2)
        micro_iterations_logger_ = std::make_shared<MicroIterationsLog>(options_,subproblem_constraint_map_,constraints_csv_map_,warm_start_) ; 
}

Benders_Jl_MICRO_ITERS::~Benders_Jl_MICRO_ITERS()
{

}

void Benders_Jl_MICRO_ITERS::OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                            const Logger& logger,
                                            const BendersBaseOptions& options,
                                            const SolverLogManager& solver_log_manager)
{
    _logger = logger;

    BuildConstraintsReaderMap(subproblem_map, options, solver_log_manager);


    if (handle_)
    {
        int size_subs = sub_pb_ids_.n_subproblems;
        for (int i = 0; i < size_subs; i++)
        {
            std::string sub_key(sub_pb_ids_.subProblems_ids[i]);
            added_constraints_per_sub_[sub_key] = std::vector<std::string>();
        }

        //reading inputs on julia side
        jl_load_variables_FUNC jl_load_variables = (jl_load_variables_FUNC)
          dlsym(handle_, "jl_load_variables");
        jl_load_variables(sub_pb_ids_);
    }
}

void Benders_Jl_MICRO_ITERS::OnBendersEnd()
{
    // std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersEnd"<<std::endl ;
    if (options_.LOG_LEVEL >= 2)
        micro_iterations_logger_->Dump() ; 

    if (handle_)
    {
        shut_down_julia_FUNC shut_down_julia = (shut_down_julia_FUNC)dlsym(handle_,
                                                                           "shutdown_julia");
        shut_down_julia(0);
        dlclose(handle_);
    }

}

void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationStart(
  std::map<std::string, double>& benders_invested_master_result,
    int& num_iter)
{
    for (auto& [sub, _]: added_constraints_per_sub_)
    {
        added_constraints_per_sub_[sub] = std::vector<std::string>();
    }

    // std::cout<<"from Benders_Jl_MICRO_ITERSOnBendersMasterIterationStart"<<std::endl ;
    std::vector<CandidateLineMasterIterationResult> candidates_iter_res ; 
    candidates_iter_res.reserve(benders_invested_master_result.size()) ;
    for (auto& [line, value]: benders_invested_master_result)
    {
        auto id_in_csv = binary_variables_ids_map_[line].c_str();
        candidates_iter_res.push_back(CandidateLineMasterIterationResult{id_in_csv, value});
    }

    MasterBendersInput master_benders_input = MasterBendersInput{
      candidates_iter_res.data(),
      benders_invested_master_result.size()};
    
    jl_compute_factors_for_microiterations_FUNC compute_factors
      = (jl_compute_factors_for_microiterations_FUNC)
        dlsym(handle_, "jl_compute_factors_for_microiterations");


    auto t1 = std::chrono::high_resolution_clock::now() ; 
    const char* jl_log_msg = compute_factors(master_benders_input,num_iter);
    auto t2 = std::chrono::high_resolution_clock::now() ; 
    auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() ;  
    if (options_.LOG_LEVEL >= 2)
        micro_iterations_logger_->AddMasterIterationLog(num_iter, std::to_string(elapsed_microseconds)) ; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationEnd()
{
    if (!warm_start_)
    {
        std::map<std::string,std::string> removing_rows_per_sub_time  ; 
        
        for (auto [sub_name,added_constraints_vec] : added_constraints_per_sub_) 
        {       
            auto t1 = std::chrono::high_resolution_clock::now() ; 
            std::string constraint_reader_name = subproblem_constraint_map_[sub_name];
            auto constraint_reader =  constraints_map_[constraint_reader_name] ; 
            
            for (auto& added_key : added_constraints_vec )
            {
                constraint_reader->delete_added_rows(constraints_csv_map_[added_key]) ; 
            }
            auto t2 = std::chrono::high_resolution_clock::now() ; 
            auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() ;
            removing_rows_per_sub_time[sub_name] = std::to_string(elapsed_microseconds) ; 
        }
        if (options_.LOG_LEVEL >= 2)
            micro_iterations_logger_->UpdateLastMasterIteration(std::move(removing_rows_per_sub_time)) ; 
        
    }
    if (options_.LOG_LEVEL >= 2)
        micro_iterations_logger_->RefreshLogger() ; 

}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationStart()
{

}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationEnd(std::string sub_name, bool& added_rows, std::string solving_time)
{
    std::string constraint_reader_name = subproblem_constraint_map_[sub_name];
    auto constraint_reader = constraints_map_[constraint_reader_name];

    jl_return_constraints_for_micro_iteration_FUNC jl_return_constraints_for_micro_iteration
      = (jl_return_constraints_for_micro_iteration_FUNC)
        dlsym(handle_, "jl_return_constraints_for_micro_iteration");
    auto sub_solution = constraint_reader->get_sub_solution();
    std::vector<FlowN> flows_to_follow;
    flows_to_follow.reserve(variables_to_follow_.size());

    for (auto& [line, line_id]: variables_to_follow_)
    {
        int variable_index = constraint_reader->get_variable_index_in_solution(line_id);
        auto value = sub_solution[variable_index];
        flows_to_follow.push_back(FlowN{line.c_str(), value});
    }

    FlowNList N_flows = FlowNList{flows_to_follow.data(), flows_to_follow.size()};
    auto t1 = std::chrono::high_resolution_clock::now() ; 

    ConstraintsToAdd constraints_to_add = jl_return_constraints_for_micro_iteration(
      sub_name.c_str(),
      N_flows);

    //for logger 
    std::vector<std::string> constraints_keys_vec ;
    for (int i=0; i<constraints_to_add.size; i++) 
    {
        std::string str(constraints_to_add.constraints[i]) ; 
        constraints_keys_vec.push_back(std::move(str)); 
    }
    
    std::vector<std::string> constraints_to_add_vec = get_constraints_to_add(constraints_to_add,
                                                                             sub_name);
    
    for (auto& constraint_to_add: constraints_to_add_vec)
    {
        constraint_reader->add_rows(constraint_to_add);
    }

    auto t2 = std::chrono::high_resolution_clock::now() ;  

    auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() ;  
    
    if (options_.LOG_LEVEL)
        micro_iterations_logger_->AddMicroIterionLog(sub_name,solving_time,std::to_string(elapsed_microseconds),constraints_keys_vec) ; 

    added_rows = constraints_to_add_vec.size();
}

void Benders_Jl_MICRO_ITERS::SetSubProblemIDs(const char** subs_ids, int n_subs)
{
    sub_ids_storage_.clear();
    sub_ids_storage_.reserve(n_subs);
    for (int i = 0; i < n_subs; i++)
    {
        sub_ids_storage_.push_back(subs_ids[i]);
    }

    sub_ids_ptrs_.resize(n_subs);
    for (int i = 0; i < n_subs; i++)
    {
        sub_ids_ptrs_[i] = sub_ids_storage_[i].c_str();
    }

    sub_pb_ids_ = SubProblemIds{sub_ids_ptrs_.data(), n_subs};
}

 std::vector<std::string> Benders_Jl_MICRO_ITERS::get_constraints_to_add(
  ConstraintsToAdd& constraints_to_add_obj,
  std::string sub_name)
{
    std::vector<std::string> constraints_to_add;

    for (int i = 0; i < constraints_to_add_obj.size; i++)
    {
        if (!check_if_constraint_key_is_added(constraints_to_add_obj.constraints[i], sub_name))
        {
            std::string constraint_key(constraints_to_add_obj.constraints[i]);

            constraints_to_add.insert(constraints_to_add.end(),
                                      constraints_csv_map_[constraint_key].begin(),
                                      constraints_csv_map_[constraint_key].end());
        }
    }
    return constraints_to_add;
}

bool Benders_Jl_MICRO_ITERS::check_if_constraint_key_is_added(const char* key, std::string sub_name)
{
    std::string key_str(key);
    bool found = (std::find(added_constraints_per_sub_[sub_name].begin(),
                            added_constraints_per_sub_[sub_name].end(),
                            key_str)
                  != added_constraints_per_sub_[sub_name].end());
    if (!found)
    {
        added_constraints_per_sub_[sub_name].push_back(key_str);
    }
    return found;
}

void Benders_Jl_MICRO_ITERS::BuildConstraintsReaderMap(const SubproblemsMapPtr& subproblem_map,
                                                       const BendersBaseOptions& options,
                                                       const SolverLogManager& solver_log_manager)
{
    for (auto& [sub, sub_worker]: subproblem_map)
    {
        std::string constraints_file_name = subproblem_constraint_map_[sub];
        auto constraints_file_path = std::filesystem::path(options.INPUTROOT)
                                     / constraints_file_name;
        constraints_map_[constraints_file_name] = std::make_shared<ConstraintsReader>(
          constraints_file_path,
          options.SOLVER_NAME,
          solver_log_manager,
          _logger,
          options.LOG_LEVEL,
          variables_dictionary_path_,
          sub_worker);
    }
}

