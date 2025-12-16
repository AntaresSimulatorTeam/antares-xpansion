#include "antares-xpansion/benders/benders_core/JuliaCodeHandler.h"
#include <boost/tokenizer.hpp>
#include <fstream>
#include <optional>
#include <iostream>

JuliaCodeHandler::JuliaCodeHandler() 
{

}

std::vector<std::string> JuliaCodeHandler::get_constraints(const std::filesystem::path& micro_iteration_data)
{
    std::vector<std::string> constraints_to_add ;
    std::ifstream constraints_to_add_file_path(micro_iteration_data);  

    if (!constraints_to_add_file_path.is_open()) 
        std::cerr<<"failed to open : "<<micro_iteration_data<<std::endl ; 
    else 
        std::cout<<"successfully opened "<<micro_iteration_data<<std::endl ; 

    std::string row ;  

    while (std::getline(constraints_to_add_file_path,row)) 
    {
        boost::char_separator<char> sep("\n"); 
        boost::tokenizer<boost::char_separator<char>> tok(row,sep) ; 
        for (const auto& r : tok) 
        {
            constraints_to_add.push_back(r) ; 
        }
    }
    
    return constraints_to_add ; 
}