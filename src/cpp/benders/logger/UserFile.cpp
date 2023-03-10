#include "logger/UserFile.h"

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

UserFile::UserFile(const std::filesystem::path &filename)
    : user_name_(LogUtils::UserName()), host_name_(LogUtils::HostName()) {
  _file.open(filename, std::ofstream::out | std::ofstream::app);
  if (_file.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
  user_name_and_host_name_ = "[" + user_name_ + "@" + host_name_ + "] ";
  line_prefix_ = user_name_and_host_name_ + LINE_PREFIX_BENDERS;
}

UserFile::~UserFile() { _file.close(); }

void UserFile::display_message(const std::string &str) {
  _file << line_prefix_ << str << std::endl;
  _file.flush();
}

void UserFile::log_at_initialization(const int it_number) {
  _file << line_prefix_ << "ITERATION " << it_number << ":" << std::endl;
  _file.flush();
}

void UserFile::log_iteration_candidates(const LogData &d) {
  _file << CandidateLog(line_prefix_).log_iteration_candidates(d);
  _file.flush();
}

void UserFile::log_master_solving_duration(double durationInSeconds) {
  _file << line_prefix_ << indent_1 << "Master solved in " << durationInSeconds
        << " s" << std::endl;
  _file.flush();
}

void UserFile::LogSubproblemsSolvingWalltime(double durationInSeconds) {
  _file << line_prefix_ << indent_1
        << "Subproblems solved in (walltime): " << durationInSeconds << " s"
        << std::endl;
  _file.flush();
}

void UserFile::LogSubproblemsSolvingCumulativeCpuTime(
    double durationInSeconds) {
  _file << line_prefix_ << indent_1
        << "Subproblems solved in (cumulative cpu): " << durationInSeconds
        << " s" << std::endl;
  _file.flush();
}

void UserFile::log_at_iteration_end(const LogData &d) {
  _file << IterationResultLog(line_prefix_).Log_at_iteration_end(d);
  _file.flush();
}

void UserFile::log_at_ending(const LogData &d) {
  const double overall_cost = d.subproblem_cost + d.invest_cost;
  _file << line_prefix_ << indent_1
        << "Total number of iterations done = " << d.it << std::endl;
  _file << line_prefix_ << indent_1 << "Best solution = it " << d.best_it
        << std::endl;
  _file.flush();

  _file << line_prefix_ << indent_1
        << " Overall cost = " << commons::create_str_million_euros(overall_cost)
        << " Me" << std::endl;
  _file.flush();
}

void UserFile::log_total_duration(double durationInSeconds) {
  _file << line_prefix_ << "Benders ran in " << durationInSeconds << " s"
        << std::endl;
  _file.flush();
}

void UserFile::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  _file << line_prefix_
        << "--- Run completed: " << criterion_to_str(stopping_criterion)
        << " reached" << std::endl;
  _file.flush();
}
void UserFile::display_restart_message() {
  _file << line_prefix_ << "Restart Study..." << std::endl;
}
void UserFile::restart_elapsed_time(const double elapsed_time) {
  _file << line_prefix_ << indent_1
        << "Elapsed time: " << format_time_str(elapsed_time) << std::endl;
}
void UserFile::number_of_iterations_before_restart(const int num_iteration) {
  _file << line_prefix_ << indent_1
        << "Number of Iterations performed: " << num_iteration << std::endl;
}
void UserFile::restart_best_iteration(const int best_iteration) {
  _file << line_prefix_ << indent_1 << "Best Iteration: " << best_iteration
        << std::endl;
}
void UserFile::restart_best_iterations_infos(
    const LogData &best_iteration_data) {
  _file << line_prefix_ << indent_1 << "Best Iteration Infos: " << std::endl;
  log_master_solving_duration(best_iteration_data.master_time);
  log_iteration_candidates(best_iteration_data);
  LogSubproblemsSolvingWalltime(best_iteration_data.subproblem_time);
  log_at_iteration_end(best_iteration_data);
}

void UserFile::LogAtInitialRelaxation() {
  _file << line_prefix_ << "--- Switch master formulation to relaxed"
        << std::endl;
  _file.flush();
}

void UserFile::LogAtSwitchToInteger() {
  _file << line_prefix_
        << "--- Relaxed gap reached, switch master formulation to integer"
        << std::endl;
  _file.flush();
}
void UserFile::number_of_sub_problem_resolved(int number) {
  _file << line_prefix_ << indent_1
        << "cumulative number of call to solver (only for subproblems) : "
        << number << std::endl;
}

}  // namespace logger
}  // namespace xpansion
