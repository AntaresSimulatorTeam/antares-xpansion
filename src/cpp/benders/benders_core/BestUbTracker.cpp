#include <antares-xpansion/benders/benders_core/BestUbTracker.h>

#include <boost/tokenizer.hpp>

BestUbTracker::BestUbTracker(mpi::communicator* world,
                                     const std::filesystem::path& file_path,
                                     const std::filesystem::path& output_root,
                                     Logger logger):
    file_stream_(file_path),
    _world(world),
    _logger(logger)
{
    output_file_ = output_root / "sub_best_ub_variables.csv";
    if (!file_stream_.is_open())
    {
        _logger->display_message("sub_variables_to_save.csv  not found ") ; 
        return;

    }

    std::string line;
    if (std::getline(file_stream_, line))
    {
        boost::escaped_list_separator<char> sep('\\', ',', '\"');
        using Tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;
        Tokenizer tok(line, sep);
        variables_to_follow_.assign(tok.begin(), tok.end());
    }

}

bool BestUbTracker::set_best_ub_solution_(double new_best_ub, int iter)
{
    if (new_best_ub <= best_ub_)
    {
        best_ub_ = new_best_ub;
        last_iteration_update_ = iter;
        return true ;
    }
    return false ; 
}

void BestUbTracker::set_variables_values(std::string sub_name,
                                             const std::shared_ptr<SubproblemWorker>& worker,
                                             int iter, 
                                             double new_ub)
{

    if (set_best_ub_solution_(new_ub,iter)) 
    {

        if (iter <= 1) [[unlikely]]
        {
            for (auto& variable: variables_to_follow_)
            {
                auto index = worker->get_variable_index(variable);
                if (index < 0)
                {
                    _logger->display_message("unable to find " + variable + " in sub_problem " + sub_name);
                }
                variables_to_follow_indices_per_sub_[sub_name].push_back(index);
            }
        }
        
        if (last_iteration_update_ == iter)
        {
            values_per_sub_[sub_name] = worker->get_solution();
        }
    }
}

void BestUbTracker::dump_values()
{
    // Gather values_per_sub_ from all ranks to rank 0
    std::vector<std::map<std::string, std::vector<double>>> gathered_values;

    mpi::gather(*_world, values_per_sub_, gathered_values, 0);

    // Also gather indices so rank 0 has indices for all subproblems
    std::vector<std::map<std::string, std::vector<int>>> gathered_indices;
    mpi::gather(*_world, variables_to_follow_indices_per_sub_, gathered_indices, 0);

    if (_world->rank() != 0)
    {
        return;
    }

    // Merge all gathered maps into values_per_sub_
    for (const auto& rank_values: gathered_values)
    {
        for (const auto& [sub_name, values]: rank_values)
        {
            values_per_sub_[sub_name] = values;
        }
    }

    for (const auto& rank_indices: gathered_indices)
    {
        for (const auto& [sub_name, indices]: rank_indices)
        {
            variables_to_follow_indices_per_sub_[sub_name] = indices;
        }
    }

    std::ofstream out(output_file_);
    if (!out.is_open())
    {
        _logger->display_message("unable to open sub_best_ub_variables.csv for writing");
        return;
    }

    // header: first column is sub name, then the followed variables
    out << "sub_name";
    for (const auto& var: variables_to_follow_)
    {
        out << "," << "\"" << var << "\"";
    }
    out << "\n";

    // one row per subproblem
    for (const auto& [sub_name, values]: values_per_sub_)
    {
        out << sub_name;
        const auto& indices = variables_to_follow_indices_per_sub_[sub_name];
        for (size_t i = 0; i < indices.size(); ++i)
        {
            out << "," << values[static_cast<size_t>(indices[i])];
        }
        out << "\n";
    }
}
