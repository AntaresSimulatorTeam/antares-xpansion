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
        _logger->display_message("sub_variables_to_save.csv  not found ");
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

bool BestUbTracker::set_best_ub_solution_(double new_best_ub)
{
    if (new_best_ub <= best_ub_)
    {
        best_ub_ = new_best_ub;
        return true;
    }
    return false;
}

void BestUbTracker::set_variables_values(std::string sub_name,
                                         const std::shared_ptr<SubproblemWorker>& worker,
                                         int iter,
                                         double new_ub)
{
    std::call_once(indices_once_flag_,
                   [this, &sub_name, &worker]()
                   {
                       for (auto& variable: variables_to_follow_)
                       {
                           auto index = worker->get_variable_index(variable);
                           if (index < 0)
                           {
                               _logger->display_message("unable to find " + variable
                                                        + " in sub_problem " + sub_name);
                           }
                           variables_to_follow_indices_.push_back(index);
                       }
                   });

    // for mutlithreaded parallel subproblem case to avoid data race on best_ub
    std::lock_guard guard(mutex_);
    if (set_best_ub_solution_(new_ub))
    {
        extract_tracked_values_(sub_name, worker);
    }
}

void BestUbTracker::extract_tracked_values_(const std::string& sub_name,
                                            const std::shared_ptr<SubproblemWorker>& worker)
{
    const auto& indices = variables_to_follow_indices_;
    auto full_solution = worker->get_solution();
    auto& tracked = values_per_sub_[sub_name];
    tracked.resize(indices.size());
    for (size_t i = 0; i < indices.size(); ++i)
    {
        tracked[i] = full_solution[static_cast<size_t>(indices[i])];
    }
}

void BestUbTracker::dump_values()
{
    std::vector<std::map<std::string, std::vector<double>>> gathered_values;
    mpi::gather(*_world, values_per_sub_, gathered_values, 0);

    if (_world->rank() != 0)
    {
        return;
    }

    for (const auto& rank_values: gathered_values)
    {
        for (const auto& [sub_name, values]: rank_values)
        {
            values_per_sub_[sub_name] = values;
        }
    }

    std::ofstream out(output_file_);
    if (!out.is_open())
    {
        _logger->display_message("unable to open sub_best_ub_variables.csv for writing");
        return;
    }

    out << "sub_name";
    for (const auto& var: variables_to_follow_)
    {
        out << "," << "\"" << var << "\"";
    }
    out << "\n";

    for (const auto& [sub_name, values]: values_per_sub_)
    {
        out << sub_name;
        for (const auto& val: values)
        {
            out << "," << val;
        }
        out << "\n";
    }
}
