//
// Created by s20217 on 21/04/2021.
//
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>
#include <map>

#include "benders_sequential_core/ILogger.h"
#include "IterationResultLog.h"


namespace xpansion{
namespace logger {

    const std::string indent_0 = "\t\t";
    const std::string indent_1 = "\t";

    const std::string LABEL = "LABEL";
    const std::string VALUE = "VALUE";
    const std::string UNIT = "UNIT";

    typedef std::map<std::string, std::string> value_map;
    typedef std::map<std::string, int> size_map;

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

    inline std::string create_solution_str(const value_map& value,
                                           const size_map& sizes)
    {
        std::stringstream result;
        result << indent_0 << indent_1 << std::setw(sizes.at(LABEL)) << value.at(LABEL);
        result << " = " << std::setw(sizes.at(VALUE))  << value.at(VALUE) << value.at(UNIT);
        return result.str();
    }

    inline value_map create_value_map(const std::string& label, const std::string& value, const std::string& unit)
    {
        value_map result;
        result[LABEL] = label;
        result[VALUE] = value;
        result[UNIT] = unit;

        return result;
    }

    std::string IterationResultLog::Log_at_iteration_end(const LogData &d) {

        std::stringstream _stream;

        //Values that need to be aligned
        typedef std::tuple<std::string, std::string,std::string> value_type;

        const double gap = d.best_ub - d.lb;
        const double overall_cost = d.slave_cost + d.invest_cost;

        //Get values
        std::list<value_map> values;
        values.push_back(create_value_map("Operational cost", create_str_million_euros(d.slave_cost)," Me") );
        values.push_back(create_value_map("Investment cost", create_str_million_euros(d.invest_cost), " Me") );
        values.push_back(create_value_map("Overall cost", create_str_million_euros(overall_cost), " Me") );
        values.push_back(create_value_map("Best Solution", create_str_million_euros(d.best_ub), " Me"));
        values.push_back(create_value_map("Lower Bound", create_str_million_euros(d.lb), " Me"));
        values.push_back(create_value_map("Gap", create_str_euros(gap)," e")  );

        //Compute maximum string size
        size_map  sizes;
        for (auto value : values) {
            sizes[LABEL] = std::max<int>(value.at(LABEL).length(), sizes[LABEL]);
            sizes[VALUE] = std::max<int>(value.at(VALUE).length(), sizes[VALUE]);
        }

        _stream << indent_0 <<"Solution =" << std::endl;
        for (auto value : values)  {
            _stream << create_solution_str(value, sizes) << std::endl;
        }

        return _stream.str();
    }

} // namespace logger
} // namespace xpansion