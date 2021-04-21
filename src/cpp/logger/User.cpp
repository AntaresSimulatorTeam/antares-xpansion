//
// Created by s20217 on 20/04/2021.
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>
#include "logger/User.h"


namespace xpansion{
namespace logger {

    User::User(std::ostream& stream)
    :_stream(stream)
    {
    }

    void User::log_at_initialization(const LogData &) {

    }

    inline std::string create_candidate_str(const std::string& candidate, int candidate_width,
                                            const std::string& investment, int investment_width,
                                            const std::string& min_investment, int min_investment_width,
                                            const std::string& max_investment, int max_investment_width)
    {
        std::stringstream result;
        result << "\t\t\t" << std::setw(candidate_width) <<  candidate << " = " << std::setw(investment_width) << investment << " invested MW -- possible interval [" << std::setw(min_investment_width) << min_investment << "; " << std::setw(max_investment_width) << max_investment << "] MW";
        return result.str();
    }

    inline std::string create_investment_str(double val)
    {
        std::stringstream result;
        result <<  std::fixed << std::setprecision(2) << val;
        return result.str();
    }

    void User::log_at_iteration_start(const LogData &d) {

        //Values that need to be aligned
        typedef std::tuple<std::string, std::string , std::string, std::string> value_type;
        typedef std::tuple<int,int,int,int> value_size_type;

        _stream << "\t\tCandidates:" << std::endl;

        //Get values
        std::list<value_type> values;
        value_size_type values_size = std::make_tuple(0,0,0,0);

        for (auto pairVarnameValue : d.x0)
        {
            std::string candidate = pairVarnameValue.first;
            value_type value = std::make_tuple(candidate,
                                               create_investment_str(pairVarnameValue.second),
                                               create_investment_str(d.min_invest.at(candidate)),
                                               create_investment_str(d.max_invest.at(candidate)));
            values.push_back(value);

            //Compute maximum string size
            std::get<0>(values_size) = std::max<int>(std::get<0>(value).length(), std::get<0>(values_size));
            std::get<1>(values_size) = std::max<int>(std::get<1>(value).length(), std::get<1>(values_size));
            std::get<2>(values_size) = std::max<int>(std::get<2>(value).length(), std::get<2>(values_size));
            std::get<3>(values_size) = std::max<int>(std::get<3>(value).length(), std::get<3>(values_size));
        }

        //Add candidates values
        for (auto value : values)
        {
            _stream << create_candidate_str(std::get<0>(value), std::get<0>(values_size),
                                            std::get<1>(value),std::get<1>(values_size),
                                            std::get<2>(value),std::get<2>(values_size),
                                            std::get<3>(value),std::get<3>(values_size)) << std::endl;
        }
    }

    inline double convert_in_million_euros(double val) { return val / 1e6;}

    inline std::string create_str_million_euros(double val)
    {
        std::stringstream result;
        result << std::fixed <<  std::setprecision(2) << convert_in_million_euros(val);
        return result.str();
    }

    inline std::string create_str_euros(double val)
    {
        std::stringstream result;
        result << std::scientific << std::setprecision(5) << val;
        return result.str();
    }

    inline std::string create_solution_str(const std::string& type,int type_width,
                                           const std::string& val,int val_width,
                                           const std::string& unit)
    {
        std::stringstream result;
        result << "\t\t\t" << std::setw(type_width) << type << " = " << std::setw(val_width)  << val << unit;
        return result.str();
    }

    void User::log_at_iteration_end(const LogData &d) {

        //Values that need to be aligned
        typedef std::tuple<std::string, std::string,std::string> value_type;
        typedef std::tuple<int,int> value_size_type;

        const double gap = d.best_ub - d.lb;
        const double overall_cost = d.slave_cost + d.invest_cost;

        //Get values
        std::list<value_type> values;
        values.push_back(std::make_tuple("Operational cost", create_str_million_euros(d.slave_cost)," Me") );
        values.push_back(std::make_tuple("Investment cost", create_str_million_euros(d.invest_cost), " Me") );
        values.push_back(std::make_tuple("Overall cost", create_str_million_euros(overall_cost), " Me") );
        values.push_back(std::make_tuple("Best Solution", create_str_million_euros(d.best_ub), " Me"));
        values.push_back(std::make_tuple("Lower Bound", create_str_million_euros(d.lb), " Me"));
        values.push_back(std::make_tuple("Gap", create_str_euros(gap)," e")  );

        //Compute maximum string size
        value_size_type values_size = std::make_tuple(0,0);
        for (auto value : values) {
            std::get<0>(values_size) = std::max<int>(std::get<0>(value).length(), std::get<0>(values_size));
            std::get<1>(values_size) = std::max<int>(std::get<1>(value).length(), std::get<1>(values_size));
        }

        _stream << "\t\tSolution =" << std::endl;
        for (auto value : values)  {
            _stream << create_solution_str(std::get<0>(value), std::get<0>(values_size),
                                           std::get<1>(value),std::get<1>(values_size),
                                           std::get<2>(value)) << std::endl;
        }
    }

    void User::log_at_ending(const LogData &d) {

        const double gap = d.best_ub - d.lb;
        const double overall_cost = d.slave_cost + d.invest_cost;

        std::string optimality = gap <= d.optimal_gap ? "within" : "outside";

        std::stringstream str;
        _stream << "--- CONVERGENCE " << optimality << " optimitality gap :" << std::endl;
        _stream << "\tBest solution = it " << d.best_it << std::endl;
        _stream << "\t Overall cost = " << create_str_million_euros(overall_cost) << " Me" << std::endl;


    }

} // namespace logger
} // namespace xpansion