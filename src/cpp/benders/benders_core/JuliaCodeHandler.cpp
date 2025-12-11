#include "antares-xpansion/benders/benders_core/JuliaCodeHandler.h"
#include <boost/tokenizer.hpp>
#include <fstream>
#include <optional>
#include <iostream>

JuliaCodeHandler::JuliaCodeHandler() 
{

}

void JuliaCodeHandler::read_csv_micro_iterations(const std::filesystem::path& micro_iteration_data) 
{
    auto rows = readCsv(micro_iteration_data) ; 
    for (auto&& row : rows)
    {
        for (auto&& field : row.fields) 
        {
            std::visit([&](auto&& value) {
                using T = std::decay_t<decltype(value)>;
                
                if constexpr (std::is_same_v<T, std::string>) 
                {
                    std::cout << "string value "<< value << std::endl;
                }
                else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                    std::cout <<  " array [ ";
                    for (auto& s : value) std::cout << s << " ";
                    std::cout << "]\n";
                }
            },field) ; 
        }
        std::cout<<"************************ \n\n ****************" <<std::endl ; 
    }
}


std::string JuliaCodeHandler::trim(const std::string& s) 
{
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);

}

bool JuliaCodeHandler::isArrayField(const std::string& field) 
{
    return field.size() >= 2 && field.front() == '[' && field.back() == ']';
}

std::vector<std::string> JuliaCodeHandler::parseArray(const std::string& field) 
{
    std::string inner = field.substr(1, field.size() - 2); 
    std::vector<std::string> result;

    boost::char_separator<char> sep(";");
    boost::tokenizer<boost::char_separator<char>> tok(inner, sep);

    for (auto& token : tok) 
    {
        std::string t = trim(token);
        if (!t.empty())
            result.push_back(t);
    }
    return result;
}

std::vector<CsvRow> JuliaCodeHandler::readCsv(const std::filesystem::path& micro_iteration_data) {
    using tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;

    boost::escaped_list_separator<char> sep('\\', ',', '\"');

    std::vector<CsvRow> rows;
    std::ifstream file(micro_iteration_data);

    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << micro_iteration_data << "\n";
        return rows;
    }

    std::string line;
    while (std::getline(file, line)) {
        tokenizer tok(line, sep);
        CsvRow row;

        for (auto& rawField : tok) {
            std::string f = trim(rawField);

            if (f.empty()) {
                row.fields.push_back(""); 
            }
            else if (isArrayField(f)) {
                row.fields.push_back(parseArray(f));
            }
            else {
                row.fields.push_back(f); 
            }
        }

        rows.push_back(std::move(row));
    }

    return rows;
}
