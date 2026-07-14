#pragma once

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

class Sub_best_ub_files
{
public:
    explicit Sub_best_ub_files(const std::filesystem::path& file_path);
    ~Sub_best_ub_files() = default;
    void set_best_ub_solution_(double new_best_ub, int iter) ; 



private:
    std::ifstream file_stream_;
    std::vector<std::string> variables_to_follow_;
    double best_ub_ = std::numeric_limits<double>::max() ; 
    double last_iteration_update_ = -1 ;
    std::map<std::string,std::vector<std::string>> 
};
