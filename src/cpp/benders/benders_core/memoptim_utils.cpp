#include "antares-xpansion/benders/benders_core/memoptim_utils.h"

#include <fstream>
#include <iostream>

#include <boost/tokenizer.hpp>

namespace memoptim_utils
{

void read_keyed_coeffs_csv(const std::filesystem::path& csv_path,
                           std::map<std::string, std::vector<double>>& dest)
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

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
        std::vector<std::string> tokens(tok.begin(), tok.end());
        std::string key = tokens[0];
        std::vector<double> values_double;
        if (tokens.size() > 1)
        {
            values_double.resize(tokens.size() - 1);
            std::transform(tokens.begin() + 1,
                           tokens.end(),
                           values_double.begin(),
                           [](const std::string& s) { return std::stod(s); });
        }
        dest[key] = values_double;
    }
}

void read_indices_csv(const std::filesystem::path& csv_path,
                      std::vector<int>& dest_indices,
                      bool is_col,
                      const std::shared_ptr<SolverAbstract>& solver)
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

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
    for (auto& name: names)
    {
        if (is_col)
        {
            dest_indices.push_back(solver->get_col_index(name));
        }
        else
        {
            dest_indices.push_back(solver->get_row_index(name));
        }
    }
}

} // namespace memoptim_utils
