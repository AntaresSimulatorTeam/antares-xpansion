//
// Created by s20217 on 20/04/2021.
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include "logger/User.h"


namespace xpansion{
namespace logger {

    User::User(std::ostream& stream)
    :_stream(stream)
    {
    }

    void User::log_at_initialization(const LogData &) {

    }

    std::string create_candidate_str(const std::string& candidate, double investment, const LogData& data, int candidate_str_width)
    {
        std::stringstream result;
        result << "\t\t\t" << std::setw(candidate_str_width) <<  candidate << " = " << std::setw(5) << std::fixed << std::setprecision(2) << investment << " invested MW -- possible interval [" << std::fixed << std::setprecision(2) << data.min_invest.at(candidate) << "; " << std::setprecision(2) << data.max_invest.at(candidate) << "] MW";
        return result.str();
    }

    void User::log_at_iteration_start(const LogData &d) {
        _stream << "\t\tCandidates:" << std::endl;

        int max_candidate_str_width = 0;
        for (auto pairVarnameValue : d.x0)
        {
            const std::string& candidate = pairVarnameValue.first;
            max_candidate_str_width = std::max<int>(candidate.length(), max_candidate_str_width);
        }
        for (auto pairVarnameValue : d.x0)
        {
            const std::string& candidate = pairVarnameValue.first;
            _stream << create_candidate_str(candidate, pairVarnameValue.second, d, max_candidate_str_width) << std::endl;
        }
    }

    void User::log_at_iteration_end(const LogData &d) {

    }

    void User::log_at_ending(const LogData &d) {

    }

} // namespace logger
} // namespace xpansion