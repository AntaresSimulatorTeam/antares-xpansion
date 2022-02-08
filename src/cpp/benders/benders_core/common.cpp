#include "common.h"

/*!
 *  \brief Return the distance between two point using 2-norm
 *
 *  \param x0 : first point
 *
 *  \param x1 : second point
 */
double norm_point(Point const &x0, Point const &x1)
{
    double result(0);
    for (auto &kvp : x0)
    {
        result += (x0.find(kvp.first)->second - x1.find(kvp.first)->second) * (x0.find(kvp.first)->second - x1.find(kvp.first)->second);
    }
    result = std::sqrt(result);
    return result;
}

LogData bendersDataToLogData(const BendersData &data)
{
    LogData result;
    result.lb = data.lb;
    result.best_ub = data.best_ub;
    result.it = data.it;
    result.best_it = data.best_it;
    result.slave_cost = data.slave_cost;
    result.invest_cost = data.invest_cost;
    result.x0 = data.x0;
    result.min_invest = data.min_invest;
    result.max_invest = data.max_invest;
    return result;
}

/*!
 *  \brief How to call for the algorithm
 *
 *  \param argc : number of arguments in command line
 */
void usage(int argc)
{
    if (argc < 2)
    {
        std::cerr << "Error: usage is : <exe> <option_file> " << std::endl;
        std::exit(1);
    }
}

/*!
 *  \brief Build the input from the structure file
 *
 *	Function to build the map linking each problem name to its variables and their id
 *
 *  \param root : root of the structure file
 *
 *  \param summary_name : name of the structure file
 *
 *  \param coupling_map : empty map to increment
 *
 *  \note The id in the coupling_map is that of the variable in the solver responsible for the creation of
 *  the structure file.
 */
CouplingMap build_input(const std::string &structure_path, const int slave_number, const std::string &master_name)
{
    CouplingMap coupling_map;
    std::ifstream summary(structure_path, std::ios::in);
    if (!summary)
    {
        // TODO JMK : gestion cas d'erreur si pas de structure d'entrée
        std::cout << "Cannot open file summary " << structure_path << std::endl;
        return coupling_map;
    }
    std::string line;

    while (std::getline(summary, line))
    {
        std::stringstream buffer(line);
        std::string problem_name;
        std::string variable_name;
        int variable_id;
        buffer >> problem_name;
        buffer >> variable_name;
        buffer >> variable_id;
        coupling_map[problem_name][variable_name] = variable_id;
    }

    if (slave_number >= 0)
    {
        int n(0);
        CouplingMap trimmer;
        for (auto const &problem : coupling_map)
        {
            if (problem.first == master_name)
                trimmer.insert(problem);
            else if (n < slave_number)
            {
                trimmer.insert(problem);
                ++n;
            }
        }
        coupling_map = trimmer;
    }
    summary.close();
    return coupling_map;
}
