#include "logger/User.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include "CandidateLog.h"
#include "Commons.h"
#include "IterationResultLog.h"

using xpansion::logger::commons::indent_1;
namespace xpansion {
namespace logger {

User::User(std::ostream &stream) : _stream(stream) {
  if (_stream.fail()) {
    std::cerr << "Invalid stream passed as parameter" << std::endl;
  }
}

void User::display_message(const std::string &str) {
  _stream << str << std::endl;
}

void User::log_at_initialization(const int it_number) {
  _stream << "ITERATION " << it_number << ":" << std::endl;
}

void User::log_iteration_candidates(const LogData &d) {
  _stream << CandidateLog().log_iteration_candidates(d);
}

void User::log_master_solving_duration(double durationInSeconds) {
  _stream << indent_1 << "Master solved in " << durationInSeconds << " s"
          << std::endl;
}

void User::log_subproblems_solving_duration(double durationInSeconds) {
  _stream << indent_1 << "Subproblems solved in " << durationInSeconds << " s"
          << std::endl;
}

void User::log_at_iteration_end(const LogData &d) {
  _stream << IterationResultLog().Log_at_iteration_end(d);
}

void User::log_at_ending(const LogData &d) {
  const double overall_cost = d.subproblem_cost + d.invest_cost;
  _stream << indent_1 << "Best solution = it " << d.best_it << std::endl;
  _stream << indent_1 << " Overall cost = "
          << commons::create_str_million_euros(overall_cost) << " Me"
          << std::endl;
}
void User::log_total_duration(double durationInSeconds) {
  _stream << "Problem ran in " << durationInSeconds << " s" << std::endl;
}

void User::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  _stream << "--- Run completed: " << criterion_to_str(stopping_criterion)
          << " reached" << std::endl;
}

}  // namespace logger
}  // namespace xpansion
