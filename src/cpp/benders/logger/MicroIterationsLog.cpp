#include "antares-xpansion/benders/logger/MicroIterationsLog.h"



MicroIterationsLog::MicroIterationsLog(const std::string& output_str, std::map<std::string,std::string>& sub_constraints_map) 
{
    root_path = output_str ; 
    sub_constraints_map_ = sub_constraints_map ; 
}


void MicroIterationsLog::AddMasterIterationLog(Point master_sol, int num_iter, std::string elapsed_time) 
{
    master_iterations_logs.push_back(MasterIterationLog{num_iter,elapsed_time,master_sol}) ; 
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


void MicroIterationsLog::RefreshLogger()  
{
    micro_iterations_per_benders_iter.push_back(std::move(micro_iter_per_sub_per_benders_iter_)) ; 
}

