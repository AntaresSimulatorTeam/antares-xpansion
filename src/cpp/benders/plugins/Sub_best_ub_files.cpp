#include <antares-xpansion/benders/plugins/Sub_best_ub_files.h>

#include <boost/tokenizer.hpp>
#include "iostream"


Sub_best_ub_files::Sub_best_ub_files(const std::filesystem::path& file_path)
    : file_stream_(file_path)
{
    if (!file_stream_.is_open()) {
        return;
    }

    std::string line;
    if (std::getline(file_stream_, line)) {
        boost::escaped_list_separator<char> sep('\\', ',', '\"');
        using Tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;
        Tokenizer tok(line, sep);
        variables_to_follow_.assign(tok.begin(), tok.end());
    }

    for (auto& variable : variables_to_follow_) 
    {
        std::cout<<"variable "<<variable<<std::endl; 
    }
}

void Sub_best_ub_files::set_best_ub_solution_(double new_best_ub, int iter)
{
    if (new_best_ub < best_ub_) 
    {
        std::cout<<"found new best ub "<<std::endl ; 
        last_iteration_update_ = iter ; 
    }
}
