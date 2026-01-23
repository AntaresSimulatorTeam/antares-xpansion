#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include "antares-xpansion/benders/benders_core/common.h"


struct MasterIterationLog 
{
    int num_iter ; 
    std::string PTDF_compute_time ; 
    std::map<std::string, double> master_solution ; 
}; 


struct MicroIterationLog
{
    std::string solving_time; 
    std::string adding_rows_time ; 
    std::vector<std::string> added_constraints_keys;  
} ; 

using MicroIterationsPerSub = std::map<std::string,std::vector<MicroIterationLog>> ; 



class MicroIterationsLog
{
    public : 
        MicroIterationsLog(const std::string& , std::map<std::string,std::string>&) ; 
        void AddMasterIterationLog(std::map<std::string, double> master_sol, int num_iter, std::string elapsed_time) ;
        void AddMicroIterionLog(std::string sub_name, std::string solving_name, 
                                    std::string adding_rows_time, 
                                    std::vector<std::string> added_constraints_keys ) ;  
        void RefreshLogger() ; 
        void Dump() ; 
    private : 
        std::filesystem::path root_path ; 
        std::vector<MasterIterationLog> master_iterations_logs_ ; 
        std::vector<MicroIterationsPerSub> micro_iterations_per_benders_iter ; 
        std::map<std::string,std::string> sub_constraints_map_ ; 
        MicroIterationsPerSub micro_iter_per_sub_per_benders_iter_ ; 

} ; 