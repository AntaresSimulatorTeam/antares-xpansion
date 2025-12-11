#pragma once 

#include <filesystem>
#include <string>
#include <vector>
#include <variant>

using CsvField = std::variant<std::string, std::vector<std::string>> ; 
struct CsvRow
{
    std::vector<CsvField> fields ; 
};
class JuliaCodeHandler 
{
    public : 
    JuliaCodeHandler() ; 
    void read_csv_micro_iterations(const std::filesystem::path& micro_iteration_data) ;
    
    private : 
    static inline std::string trim(const std::string& s) ; 
    bool isArrayField(const std::string& field) ; 
    std::vector<std::string> parseArray(const std::string& field) ; 
    std::vector<CsvRow> readCsv(const std::filesystem::path& micro_iteration_data) ; 

} ;


