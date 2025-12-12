#pragma once 

#include <filesystem>
#include <string>
#include <vector>

class JuliaCodeHandler 
{
    public : 
    JuliaCodeHandler() ; 
    std::vector<std::string> get_constraints(const std::filesystem::path& micro_iteration_data) ;
        
} ;


