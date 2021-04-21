//
// Created by s20217 on 21/04/2021.
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>
#include <map>

#include "benders_sequential_core/ILogger.h"
#include "CandidateLog.h"

namespace xpansion{
namespace logger {

    const std::string indent_0 = "\t\t";
    const std::string indent_1 = "\t";

    const std::string CANDIDATE = "CANDIDATE";
    const std::string INVEST = "INVEST";
    const std::string INVEST_MIN = "INVEST_MIN";
    const std::string INVEST_MAX = "INVEST_MAX";

    typedef std::map<std::string, std::string> value_map;
    typedef std::map<std::string, int> size_map;

    inline std::string create_candidate_str(const value_map &values,
                                            const size_map &sizes) {
        std::stringstream result;
        result << indent_0 << indent_1 << std::setw(sizes.at(CANDIDATE)) << values.at(CANDIDATE);
        result << " = " << std::setw(sizes.at(INVEST)) << values.at(INVEST) << " invested MW ";
        result << "-- possible interval [" << std::setw(sizes.at(INVEST_MIN)) << values.at(INVEST_MIN);
        result << "; " << std::setw(sizes.at(INVEST_MAX)) << values.at(INVEST_MAX) << "] MW";
        return result.str();
    }

    inline std::string create_investment_str(double val) {
        std::stringstream result;
        result << std::fixed << std::setprecision(2) << val;
        return result.str();
    }

    std::string CandidateLog::Log_at_iteration_start(const LogData &d) {
        std::stringstream _stream;
        _stream << indent_0 << "ITERATION " << d.it << ":" << std::endl;
        _stream << indent_0 << "Candidates:" << std::endl;

        //Get values
        std::list<value_map> values;
        size_map sizes;

        for (auto pairVarnameValue : d.x0) {
            value_map valuesMap;
            std::string candidate = pairVarnameValue.first;

            valuesMap[CANDIDATE] = candidate;
            valuesMap[INVEST] = create_investment_str(pairVarnameValue.second);
            valuesMap[INVEST_MIN] = create_investment_str(d.min_invest.at(candidate));
            valuesMap[INVEST_MAX] = create_investment_str(d.max_invest.at(candidate));

            values.push_back(valuesMap);

            //Compute maximum string size
            for (auto it : valuesMap) {
                const std::string &key = it.first;
                sizes[key] = std::max<int>(it.second.length(), sizes[key]);
            }
        }

        //Add candidates values
        for (auto value : values) {
            _stream << create_candidate_str(value, sizes) << std::endl;
        }
        return _stream.str();
    }

} // namespace logger
} // namespace xpansion