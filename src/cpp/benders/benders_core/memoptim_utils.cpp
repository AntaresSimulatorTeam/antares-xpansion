#include "antares-xpansion/benders/benders_core/memoptim_utils.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/tokenizer.hpp>

MemoptimUtils::MemoptimUtils(std::vector<std::string>&& sub_problem_names):
    my_subs_(std::make_move_iterator(sub_problem_names.begin()),
             std::make_move_iterator(sub_problem_names.end()))
{
}

void MemoptimUtils::read_keyed_coeffs_csv(const std::filesystem::path& csv_path,
                                          std::map<std::string, std::vector<double>>& dest)
{
    boost::escaped_list_separator<char> sep('\\', ',', '\"');
    using Tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;

    std::ifstream stream(csv_path);
    if (!stream.is_open())
    {
        std::cerr << "Error: unable to open file " << csv_path << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(stream, line))
    {
        Tokenizer tok(line, sep);
        auto it = tok.begin();
        std::string key = *it;
        if (!my_subs_.count(key))
        {
            continue;
        }
        std::vector<double> values_double;
        ++it;
        if (it != tok.end())
        {
            std::vector<std::string> value_tokens(it, tok.end());
            values_double.resize(value_tokens.size());
            std::transform(value_tokens.begin(),
                           value_tokens.end(),
                           values_double.begin(),
                           [](const std::string& s) { return std::stod(s); });
        }
        dest[key] = std::move(values_double);
    }
}

void dump_not_found(const std::vector<std::string>& names, const std::string& filename)
{
    std::ofstream out(filename);
    for (const auto& name: names)
    {
        out << name << "\n";
    }
}

void MemoptimUtils::read_indices_csv(const std::filesystem::path& csv_path,
                                     std::vector<int>& dest_indices,
                                     bool is_col,
                                     const std::shared_ptr<SolverAbstract>& solver)
{
    boost::escaped_list_separator<char> sep('\\', ',', '\"');
    using Tokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;

    std::ifstream stream(csv_path);
    if (!stream.is_open())
    {
        std::cerr << "Error: unable to open file " << csv_path << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    std::vector<std::string> names;
    while (std::getline(stream, line))
    {
        Tokenizer tok(line, sep);
        names.assign(tok.begin(), tok.end());
    }
    int error_cols(0);
    int error_rows(0);
    std::vector<std::string> cols_not_found;
    std::vector<std::string> rows_not_found;

    for (auto& name: names)
    {
        if (is_col)
        {
            auto col_pos = solver->get_col_index(name);
            if (col_pos < 0)
            {
                error_cols++;
                cols_not_found.push_back(name);
            }
            dest_indices.push_back(solver->get_col_index(name));
        }
        else
        {
            auto row_pos = solver->get_row_index(name);
            if (row_pos < 0)
            {
                error_rows++;
                rows_not_found.push_back(name);
            }
            dest_indices.push_back(solver->get_row_index(name));
        }
    }

    if (error_cols > 0)
    {
        dump_not_found(cols_not_found, "cols_not_found.txt");
        throw std::runtime_error("Error: " + std::to_string(error_cols)
                                 + " column(s) not found while reading " + csv_path.string()
                                 + " (see cols_not_found.txt)");
    }
    if (error_rows > 0)
    {
        dump_not_found(rows_not_found, "rows_not_found.txt");
        throw std::runtime_error("Error: " + std::to_string(error_rows)
                                 + " row(s) not found while reading " + csv_path.string()
                                 + " (see rows_not_found.txt)");
    }
}
