#include "logger/User.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include "CandidateLog.h"
#include "Commons.h"
#include "IterationResultLog.h"
#include "LogUtils.h"
#include "Timer.h"

using xpansion::logger::commons::indent_1;
namespace xpansion {
namespace logger {

User::User(std::ostream &stream)
    : _stream(stream),
      user_name_(LogUtils::UserName()),
      host_name_(LogUtils::HostName()) {
  user_name_and_host_name_ = "[" + user_name_ + "@" + host_name_ + "] ";
  if (_stream.fail()) {
    std::cerr << "Invalid stream passed as parameter" << std::endl;
  }
}

void User::display_message(const std::string &str) {
  _stream << user_name_and_host_name_ << str << std::endl;
}

void User::log_at_initialization(const int it_number) {
  _stream << user_name_and_host_name_ << "ITERATION " << it_number << ":"
          << std::endl;
}

void User::log_iteration_candidates(const LogData &d) {
  _stream << CandidateLog(user_name_and_host_name_).log_iteration_candidates(d);
}

void User::log_master_solving_duration(double durationInSeconds) {
  _stream << user_name_and_host_name_ << indent_1 << "Master solved in "
          << durationInSeconds << " s" << std::endl;
}

void User::LogSubproblemsSolvingWalltime(double durationInSeconds) {
  _stream << user_name_and_host_name_ << indent_1
          << "Subproblems solved in (walltime): " << durationInSeconds << " s"
          << std::endl;
}

void User::LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds) {
  _stream << user_name_and_host_name_ << indent_1
          << "Subproblems solved in (cumulative cpu): " << durationInSeconds
          << " s" << std::endl;
}

void User::log_at_iteration_end(const LogData &d) {
  _stream
      << IterationResultLog(user_name_and_host_name_).Log_at_iteration_end(d);
}

void User::log_at_ending(const LogData &d) {
  const double overall_cost = d.subproblem_cost + d.invest_cost;
  _stream << user_name_and_host_name_ << indent_1
          << "Total number of iterations done = " << d.it << std::endl;
  _stream << user_name_and_host_name_ << indent_1 << "Best solution = it "
          << d.best_it << std::endl;
  _stream << user_name_and_host_name_ << indent_1 << " Overall cost = "
          << commons::create_str_million_euros(overall_cost) << " Me"
          << std::endl;
}
void User::log_total_duration(double durationInSeconds) {
  _stream << user_name_and_host_name_ << "Benders ran in " << durationInSeconds
          << " s" << std::endl;
}

void User::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  _stream << user_name_and_host_name_
          << "--- Run completed: " << criterion_to_str(stopping_criterion)
          << " reached" << std::endl;
}
void User::display_restart_message() {
  _stream << user_name_and_host_name_ << "Restart Study..." << std::endl;
}
void User::restart_elapsed_time(const double elapsed_time) {
  _stream << user_name_and_host_name_ << indent_1
          << "Elapsed time: " << format_time_str(elapsed_time) << std::endl;
}
void User::number_of_iterations_before_restart(const int num_iteration) {
  _stream << user_name_and_host_name_ << indent_1
          << "Number of Iterations performed: " << num_iteration << std::endl;
}
void User::restart_best_iteration(const int best_iteration) {
  _stream << user_name_and_host_name_ << indent_1
          << "Best Iteration: " << best_iteration << std::endl;
}
void User::restart_best_iterations_infos(const LogData &best_iteration_data) {
  _stream << user_name_and_host_name_ << indent_1
          << "Best Iteration Infos: " << std::endl;
  log_master_solving_duration(best_iteration_data.master_time);
  log_iteration_candidates(best_iteration_data);
  LogSubproblemsSolvingWalltime(best_iteration_data.subproblem_time);
  log_at_iteration_end(best_iteration_data);
}

void User::LogAtInitialRelaxation() {
  _stream << user_name_and_host_name_
          << "--- Switch master formulation to relaxed" << std::endl;
}

void User::LogAtSwitchToInteger() {
  _stream << user_name_and_host_name_
          << "--- Relaxed gap reached, switch master formulation to integer"
          << std::endl;
}
void User::number_of_sub_problem_resolved(int number) {
  _stream << user_name_and_host_name_ << indent_1
          << "cumulative number of subproblem resolutions: " << number
          << std::endl;
}
}  // namespace logger
}  // namespace xpansion
