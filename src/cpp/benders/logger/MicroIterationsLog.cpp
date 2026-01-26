#include "antares-xpansion/benders/logger/MicroIterationsLog.h"
#include "iostream"
#include <fstream>
#include <sstream>




MicroIterationsLog::MicroIterationsLog(const SimulationOptions& options, std::map<std::string,std::string>& sub_constraints_map,std::map<std::string,std::vector<std::string>>&constraints_per_line,bool warm_start)      
                : options_(options), constraints_per_line_(constraints_per_line) 
{
    sub_constraints_map_ = sub_constraints_map ;
    warm_start_ = warm_start ; 
}


void MicroIterationsLog::AddMasterIterationLog(int num_iter, std::string elapsed_time) 
{
    MasterIterationLog master_iteration_log ; 
    master_iteration_log.num_iter = num_iter ; 
    master_iteration_log.PTDF_compute_time = elapsed_time ; 

    master_iterations_logs_.push_back(std::move(master_iteration_log)) ; 
    for (auto& [sub_name,_] : sub_constraints_map_) 
    {
        micro_iter_per_sub_per_benders_iter_[sub_name] = std::vector<MicroIterationLog>() ; 
    }

}

void MicroIterationsLog::AddMicroIterionLog(std::string sub_name, std::string solving_time, 
                                    std::string adding_rows_time, 
                                    std::vector<std::string> added_constraints_keys )  
{
    micro_iter_per_sub_per_benders_iter_[sub_name].push_back(MicroIterationLog{
         solving_time, adding_rows_time, added_constraints_keys 
    }) ; 
}


void MicroIterationsLog::UpdateLastMasterIteration(std::map<std::string,std::string > && removing_rows_per_sub_time) 
{
    master_iterations_logs_[master_iterations_logs_.size()-1].removing_rows_per_sub_time = removing_rows_per_sub_time ;
} 


void MicroIterationsLog::RefreshLogger()  
{
    micro_iterations_per_benders_iter.push_back(std::move(micro_iter_per_sub_per_benders_iter_)) ; 
}

void MicroIterationsLog::Dump()
{
        
        std::filesystem::path micro_iterations_log_path = std::filesystem::path(options_.OUTPUTROOT)  / "micro_iterations.log" ; 
        std::ofstream micro_iterations_log_stream(micro_iterations_log_path.c_str()) ; 
        
        micro_iterations_log_stream<<"************************** MICRO ITERS config ************************** \n\n" ; 
        if (warm_start_)
            micro_iterations_log_stream<<"warl_start=1\n\n"; 
        else 
            micro_iterations_log_stream<<"warm_start=0\n\n"; 

        
        
        for (size_t i=0; i<master_iterations_logs_.size(); i++)
        {
            micro_iterations_log_stream<<"Master Iteration : "<<master_iterations_logs_[i].num_iter<<" PTDF compute time : "
            <<master_iterations_logs_[i].PTDF_compute_time<<"\n" ; 
            
            if (!warm_start_)
            {
                micro_iterations_log_stream<<"deleting rows time per subproblems\n" ;
                micro_iterations_log_stream<<"sub       time(us)\n" ; 
                for (auto& [sub_name,remove_time] : master_iterations_logs_[i].removing_rows_per_sub_time) 
                {
                    micro_iterations_log_stream<<sub_name<<"        "<<remove_time<<"\n" ; 
                }   
            }
            
            
            micro_iterations_log_stream<<"\n************************** MICRO ITERS INFOS *********************\n\n" ; 
            micro_iterations_log_stream<<"Micro iter         solving time(us)       n added         adding time(us) \n" ; 

            
            auto micro_iterations_per_master_iter_per_sub = micro_iterations_per_benders_iter[i] ; 
            
            for (auto& [sub_name, micro_iters_vec] : micro_iterations_per_master_iter_per_sub)
            {  
                size_t start = sub_name.find_last_of('/') + 1;
                size_t end = sub_name.find(".mps");
                std::string result = sub_name.substr(start, end - start); 
                micro_iterations_log_stream<<sub_name<<"\n" ;               
                for (size_t j=0; j<micro_iters_vec.size(); j++) 
                {
                    int added_constraints(0) ; 
                    // std::string file_name = "added_contraints_master_iter_" + std::to_string(i)+"_micro_iter_" + std::to_string(j) + "_" +result + ".txt" ; 
                    // std::filesystem::path added_constraints_path = std::filesystem::path(options_.OUTPUTROOT)   /  "addeded_constraints" /file_name ; 
                    // std::ofstream added_constraints_stream(added_constraints_path.c_str())  ; 
                    for (auto& added_constraints_key : micro_iters_vec[j].added_constraints_keys) 
                    {
                        added_constraints += constraints_per_line_[added_constraints_key].size() ; 
                        // for (auto& added_constraint : constraints_per_line_[added_constraints_key]) 
                        // {
                        //     added_constraints_stream<<added_constraint<<"\n" ; 
                        // }
                    }
                    // added_constraints_stream.close() ; 

                    int num_micro_iter = j + 1 ;
                    micro_iterations_log_stream<<num_micro_iter<<"      "<<micro_iters_vec[j].solving_time
                                                <<"         "<<added_constraints<<"         "
                                                <<micro_iters_vec[j].adding_rows_time<<"\n" ;
                     

                }
            }
            micro_iterations_log_stream<<"\n\n" ; 
        }
        micro_iterations_log_stream.close() ; 
}


