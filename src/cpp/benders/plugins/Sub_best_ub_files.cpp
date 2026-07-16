#include <antares-xpansion/benders/plugins/Sub_best_ub_files.h>

#include <boost/tokenizer.hpp>
#include "iostream"


Sub_best_ub_files::Sub_best_ub_files(mpi::communicator* world,const std::filesystem::path& file_path, const std::string& output_root)
    : file_stream_(file_path), _world(world)
{
    output_file_ = std::filesystem::path(output_root) / "sub_best_ub_variables.csv" ; 
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

void Sub_best_ub_files::set_variables_values(std::string sub_name,const std::shared_ptr<SubproblemWorker>& worker, int iter) 
{
    if (variables_to_follow_indices_per_sub_.find(sub_name) == variables_to_follow_indices_per_sub_.end())
    {
        std::cout<<"getting indices for "<<sub_name<<std::endl ; 
        for (auto& variable : variables_to_follow_) 
        {
            auto index = worker->get_variable_index(variable) ; 
            if (index < 0)
                std::cerr<<"unable to find "<<variable<<" in sub_problem "<<sub_name<<std::endl ; 
            variables_to_follow_indices_per_sub_[sub_name].push_back(index) ; 
        }
    }


    if (last_iteration_update_ == iter) 
    {
        std::cout<<"updating the solution "<<std::endl ; 
        values_per_sub_[sub_name] = worker->get_solution() ; 
    }
}
   
void Sub_best_ub_files::dump_values()
{
    // Gather values_per_sub_ from all ranks to rank 0
    std::vector<std::map<std::string, std::vector<double>>> gathered_values;
    
    mpi::gather(*_world, values_per_sub_, gathered_values, 0);

    // Also gather indices so rank 0 has indices for all subproblems
    std::vector<std::map<std::string, std::vector<int>>> gathered_indices;
    mpi::gather(*_world, variables_to_follow_indices_per_sub_, gathered_indices, 0);

    if (_world->rank() != 0) {
        return;
    }

    // Merge all gathered maps into values_per_sub_
    for (const auto& rank_values : gathered_values) {
        for (const auto& [sub_name, values] : rank_values) {
            values_per_sub_[sub_name] = values;
        }
    }

    for (const auto& rank_indices : gathered_indices) {
        for (const auto& [sub_name, indices] : rank_indices) {
            variables_to_follow_indices_per_sub_[sub_name] = indices;
        }
    }

    std::ofstream out(output_file_);
    if (!out.is_open()) {
        std::cerr << "unable to open sub_best_ub_variables.csv for writing" << std::endl;
        return;
    }

    // header: first column is sub name, then the followed variables
    out << "sub_name";
    for (const auto& var : variables_to_follow_) {
        out << "," << "\""<<var<<"\"";
    }
    out << "\n";

    // one row per subproblem
    for (const auto& [sub_name, values] : values_per_sub_) {
        out << sub_name;
        const auto& indices = variables_to_follow_indices_per_sub_[sub_name];
        for (size_t i = 0; i < indices.size(); ++i) {
            out << "," << values[indices[i]];
        }
        out << "\n";
    }
}

