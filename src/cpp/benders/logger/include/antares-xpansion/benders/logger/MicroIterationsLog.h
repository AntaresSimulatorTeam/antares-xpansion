// Copyright (C) 2026 Hedi Bouchehda.
//This header file will contain all the necessary objects to handle micro iterations workflow loggin

#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"


/*
This structure contains the master iteration data that we want want to log 
    - num_iter : number of master iteration
    - PTDF_compute_time : elapsed time to compute the new PTDF
    - removing_rows_per_sub_time : elapsed time to remove the constraints added to subproblem workers 
*/
struct MasterIterationLog 
{
    int num_iter ; 
    std::string PTDF_compute_time ; 
    std::map<std::string,std::string> removing_rows_per_sub_time; 
}; 


/*
This structure contains the microiteration data that we want to log 
    - solving_time : elpased time to solve the sub problem
    - adding_rows_time : elapsed time to add violated constraints 
    - added_constraints_keys : an array of the keys of constraints that has been added to the subproblems
*/
struct MicroIterationLog
{
    std::string solving_time; 
    std::string adding_rows_time ; 
    std::vector<std::string> added_constraints_keys;  
} ; 

/*
This map will be used will be attached to each master iteration. 
we map each subproblem into the different micoiteration during a benders iteration
*/
using MicroIterationsPerSub = std::map<std::string,std::vector<MicroIterationLog>> ; 



class MicroIterationsLog
{
    public : 

        /*
            Constructor
            @input : 
                - options : configuration of the study 
                - sub_constraints_map : mapping sub to constraints
                - constraints_per_line : mapping constraint keys to the list of constraints to add 
                - warm_start : handle warm_start mechanism 

        */
        MicroIterationsLog(const SimulationOptions& options, std::map<std::string,std::string>& sub_constraints_map, std::map<std::string,std::vector<std::string>>& constraints_per_line, bool warm_start) ; 

        /*
            Called in the benders master iteration start callback. 
            It will set the number of iteration and the time to compute the new PTDF. 
            @input 
                - num_iter : number of master iteration
                - elapsed_time : elapsed time of computing PTDF
        */
        void AddMasterIterationLog(int num_iter, std::string elapsed_time) ;
        
        /*
            Called in the benders master iteration end callback. 
            It will set the elapsed time of removing added constraints after the microiterations for each subproblem
            @input : 
                - removing_rows_per_sub_time : time to remove added constraints per subproblem
        */
        void UpdateLastMasterIteration(std::map<std::string,std::string > && removing_rows_per_sub_time) ; 
        
        /*
            Called in the micro iteration end callback. 
            It will set the output data we want to dump for micro iterations for each subproblem and 
            at each master iteration. 
            @inputs : 
                - sub_name : subproblem name 
                - solving_name : time to solve subproblem
                - adding_rows_time : elapsed time to add constraints
                - added_constraints_keys : keys of added constraints
        */
        void AddMicroIterionLog(std::string sub_name, std::string solving_name, 
                                    std::string adding_rows_time, 
                                    std::vector<std::string> added_constraints_keys ) ;  

        /*
            Called in the benders master iteration end callback. 
            It will reset the logger for the next master iteration
        */
        void RefreshLogger() ; 

        /*
            Called in benders end callback. 
            It will write all logged data in micro_iterations.log
        */
        void Dump() ; 
    private : 
        const SimulationOptions& options_; 
        bool warm_start_ ; 
        std::map<std::string,std::vector<std::string>>& constraints_per_line_ ; 
        std::vector<MasterIterationLog> master_iterations_logs_ ; 
        std::vector<MicroIterationsPerSub> micro_iterations_per_benders_iter ; 
        std::map<std::string,std::string> sub_constraints_map_ ; 
        MicroIterationsPerSub micro_iter_per_sub_per_benders_iter_ ; 

} ; 