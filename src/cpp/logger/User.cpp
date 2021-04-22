//
// Created by s20217 on 20/04/2021.
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>

#include "logger/User.h"
#include "CandidateLog.h"
#include "IterationResultLog.h"
#include "Commons.h"

namespace xpansion{
namespace logger {

    User::User(std::ostream& stream)
    :_stream(stream)
    {
    }

    void User::log_at_initialization(const LogData &) {

    }

    void User::log_at_iteration_start(const LogData &d) {
        _stream << CandidateLog().Log_at_iteration_start(d);
    }

    void User::log_at_iteration_end(const LogData &d) {
        _stream << IterationResultLog().Log_at_iteration_end(d);
    }

    const std::string indent_0 = "\t\t";
    const std::string indent_1 = "\t";



    void User::log_at_ending(const LogData &d) {

        const double gap = d.best_ub - d.lb;
        const double overall_cost = d.slave_cost + d.invest_cost;

        std::string optimality = gap <= d.optimal_gap ? "within" : "outside";

        std::stringstream str;
        _stream << "--- CONVERGENCE " << optimality << " optimitality gap :" << std::endl;
        _stream << indent_1 << "Best solution = it " << d.it << std::endl;
        _stream << indent_1 << " Overall cost = " << commons::create_str_million_euros(overall_cost) << " Me" << std::endl;
    }

} // namespace logger
} // namespace xpansion
