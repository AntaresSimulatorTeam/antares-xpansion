#include <algorithm>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <list>

#include "logger/User.h"
#include "CandidateLog.h"
#include "IterationResultLog.h"
#include "Commons.h"

namespace xpansion{
namespace logger {

    const std::string indent_0 = "\t\t";
    const std::string indent_1 = "\t";

    User::User(std::ostream& stream)
    :_stream(stream)
    {
        if (_stream.fail())
        {
            std::cerr << "Invalid stream passed as parameter" << std::endl;
        }
    }

    void User::display_message(const std::string& str) {
        _stream << str << std::endl;
    }

    void User::log_at_initialization(const LogData &d) {
        _stream << "ITERATION " << d.it << ":" << std::endl;
    }

    void User::log_iteration_candidates(const LogData &d) {
        _stream << CandidateLog().log_iteration_candidates(d);
    }

    void User::log_master_solving_duration(double durationInSeconds) {
        _stream << indent_1 << "Master solved in " << durationInSeconds << " s"  << std::endl;
    }

    void User::log_subproblems_solving_duration(double durationInSeconds) {
        _stream << indent_1 << "Subproblems solved in " << durationInSeconds << " s"  << std::endl;
    }

    void User::log_at_iteration_end(const LogData &d) {
        _stream << IterationResultLog().Log_at_iteration_end(d);
    }

    void User::log_at_ending(const LogData &d) {

        const double abs_gap = d.best_ub - d.lb;
        const double rel_gap = abs_gap / d.best_ub;
        const double overall_cost = d.slave_cost + d.invest_cost;

        std::string stop_crit;
        if (abs_gap <= d.optimality_gap)
        {
            stop_crit = "absolute gap reached";
        }
        else if (rel_gap <= d.relative_gap)
        {
            stop_crit = "relative gap reached";
        }
        else if (d.max_iterations != -1 && d.it > d.max_iterations)
        {
            stop_crit = "maximum iterations reached";
        }
        else
        {
            stop_crit = "unexpected termination criterion";
        }

        _stream << "--- Run completed: " << stop_crit << std::endl;
        _stream << indent_1 << "Best solution = it " << d.best_it << std::endl;
        _stream << indent_1 << " Overall cost = " << commons::create_str_million_euros(overall_cost) << " Me" << std::endl;
    }
    void User::log_total_duration(double durationInSeconds) {
        _stream << "Problem ran in " << durationInSeconds << " s"  << std::endl;
    }

} // namespace logger
} // namespace xpansion
