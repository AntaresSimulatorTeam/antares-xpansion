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

    std::string IterationResultLog::Log_at_iteration_end(const LogData &data) {
        setValuesFromData(data);
        setMaximumStringSizes();
        return getCompleteMessageString();
    }

    void IterationResultLog::setValuesFromData(const LogData &data) {
        const double gap = data.best_ub - data.lb;
        const double overall_cost = data.slave_cost + data.invest_cost;

        //Get values

        _values.clear();
        _values.push_back(create_value_map("Operational cost", create_str_million_euros(data.slave_cost), " Me") );
        _values.push_back(create_value_map("Investment cost", create_str_million_euros(data.invest_cost), " Me") );
        _values.push_back(create_value_map("Overall cost", create_str_million_euros(overall_cost), " Me") );
        _values.push_back(create_value_map("Best Solution", create_str_million_euros(data.best_ub), " Me"));
        _values.push_back(create_value_map("Lower Bound", create_str_million_euros(data.lb), " Me"));
        _values.push_back(create_value_map("Gap", create_str_euros(gap), " e")  );
    }


    void IterationResultLog::setMaximumStringSizes() {//Compute maximum string size

        for (auto value : _values) {
            _max_sizes[LABEL] = std::max<int>(value.at(LABEL).length(), _max_sizes[LABEL]);
            _max_sizes[VALUE] = std::max<int>(value.at(VALUE).length(), _max_sizes[VALUE]);
        }
    }

    std::string IterationResultLog::getCompleteMessageString() {
        std::stringstream _stream;
        _stream << indent_0 << "Solution =" << std::endl;
        for (const auto& value : _values)  {
            _stream << create_solution_str(value, _max_sizes) << std::endl;
        }
        return _stream.str();
    }

    inline std::string IterationResultLog::create_solution_str(const value_map& value,
                                                               const size_map& sizes)
    {
        std::stringstream result;
        result << indent_0 << indent_1 << std::setw(sizes.at(LABEL)) << value.at(LABEL);
        result << " = " << std::setw(sizes.at(VALUE))  << value.at(VALUE) << value.at(UNIT);
        return result.str();
    }






    inline value_map IterationResultLog::create_value_map(const std::string& label, const std::string& value, const std::string& unit)
    {
        value_map result;
        result[LABEL] = label;
        result[VALUE] = value;
        result[UNIT] = unit;

        return result;
    }


} // namespace logger
} // namespace xpansion