#pragma once


#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <map>


//This class aim to dump the results of some followed variable (like battery investments)
// at the bestub found for all the considerd subproblems 
// at each master iteration we check the new best ub if it's smaller than the one we store
// we take the value of the followed variables for each subproblem 

// The way is inserted in benders wrokflow is kind of messy. Such class fit very well as a plugin behaviour 
// This will need some refactoring of the BendersPluginFactory so we would be able to create multiple plugins in the same time 

class Sub_best_ub_files
{
public:
    explicit Sub_best_ub_files(const std::filesystem::path& file_path, const std::string& output_root="./");
    ~Sub_best_ub_files() = default;
    void set_best_ub_solution_(double new_best_ub, int iter) ; 
    void set_variables_values(std::string sub_name,const std::shared_ptr<SubproblemWorker>& worker, int iter) ; 
    void dump_values() ; 

private:
    std::ifstream file_stream_;
    std::filesystem::path output_file_ ; 
    std::vector<std::string> variables_to_follow_;
    std::map<std::string,std::vector<int>> variables_to_follow_indices_per_sub_ ;  
    double best_ub_ = std::numeric_limits<double>::max() ; 
    double last_iteration_update_ = -1 ;
    std::map<std::string,std::vector<double>> values_per_sub_ ; 

};
