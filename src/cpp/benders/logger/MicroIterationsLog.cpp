#include "antares-xpansion/benders/logger/MicroIterationsLog.h"
#include "iostream"



MicroIterationsLog::MicroIterationsLog(const std::string& output_str, std::map<std::string,std::string>& sub_constraints_map) 
{
    root_path = output_str ; 
    sub_constraints_map_ = sub_constraints_map ; 
}


void MicroIterationsLog::AddMasterIterationLog(Point master_sol, int num_iter, std::string elapsed_time) 
{
    master_iterations_logs_.push_back(MasterIterationLog{num_iter,elapsed_time,master_sol}) ; 
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

void MicroIterationsLog::Dump()
{
    for (size_t i=0; i<master_iterations_logs_.size(); i++)
    {
        std::cout<<"Master iteration num : "<<master_iterations_logs_[i].num_iter
                <<" PTDF compute time "<<master_iterations_logs_[i].PTDF_compute_time<<std::endl ; 

        auto micro_iterations_per_master_iter_per_sub = micro_iterations_per_benders_iter[i] ; 
        std::cout<<"\n"<<std::endl ;
        std::cout<<"************************** MICRO ITERS INFOS *********************"<<std::endl ; 
        std::cout<<"\n"<<std::endl ; 
        for (auto& [sub_name, micro_iters_vec] : micro_iterations_per_master_iter_per_sub)
        {
            std::cout<<"sub name "<<sub_name<<" num of micro iterations "<<micro_iters_vec.size()<<std::endl ; 
            for (size_t j=0; j<micro_iters_vec.size(); j++) 
            {
                std::cout<<"solving time "<<micro_iters_vec[j].solving_time
                        <<" adding rows time "<<micro_iters_vec[j].adding_rows_time 
                        <<" added constraints keys size "<<micro_iters_vec[j].added_constraints_keys.size()<<std::endl ; 
            
                std::cout<<"------------"<<std::endl ; 
            }

        }
        
 
        
    }
    std::cout<<"\n\n"<<std::endl ; 
}


