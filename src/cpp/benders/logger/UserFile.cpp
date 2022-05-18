#include "logger/UserFile.h"

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

UserFile::UserFile(const std::filesystem::path &filename) {
  _file.open(filename, std::ofstream::out | std::ofstream::app);
  if (_file.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
}

UserFile::~UserFile() { _file.close(); }

void UserFile::display_message(const std::string &str) {
  _file << LINE_PREFIX << str << std::endl;
  _file.flush();
}

void UserFile::log_at_initialization(const int it_number) {
  _file << LINE_PREFIX << "ITERATION " << it_number << ":" << std::endl;
  _file.flush();
}

void UserFile::log_iteration_candidates(const LogData &d) {
  _file << CandidateLog(LINE_PREFIX).log_iteration_candidates(d);
  _file.flush();
}

void UserFile::log_master_solving_duration(double durationInSeconds) {
  _file << LINE_PREFIX << indent_1 << "Master solved in " << durationInSeconds
        << " s" << std::endl;
  _file.flush();
}

void UserFile::log_subproblems_solving_duration(double durationInSeconds) {
  _file << LINE_PREFIX << indent_1 << "Subproblems solved in "
        << durationInSeconds << " s" << std::endl;
  _file.flush();
}

void UserFile::log_at_iteration_end(const LogData &d) {
  _file << IterationResultLog(LINE_PREFIX).Log_at_iteration_end(d);
  _file.flush();
}

void UserFile::log_at_ending(const LogData &d) {
  const double overall_cost = d.subproblem_cost + d.invest_cost;
  _file << LINE_PREFIX << indent_1 << "Best solution = it " << d.best_it
        << std::endl;
  _file.flush();

  _file << LINE_PREFIX << indent_1
        << " Overall cost = " << commons::create_str_million_euros(overall_cost)
        << " Me" << std::endl;
  _file.flush();
}

void UserFile::log_total_duration(double durationInSeconds) {
  _file << LINE_PREFIX << "Problem ran in " << durationInSeconds << " s"
        << std::endl;
  _file.flush();
}

void UserFile::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  _file << LINE_PREFIX
        << "--- Run completed: " << criterion_to_str(stopping_criterion)
        << " reached" << std::endl;
  _file.flush();
}
void UserFile::display_restart_message() {
  _file << LINE_PREFIX << "Restart Study..." << std::endl;
}
void UserFile::restart_elapsed_time(const double elapsed_time) {
  _file << LINE_PREFIX << indent_1
        << "Elapsed time: " << format_time_str(elapsed_time) << std::endl;
}
void UserFile::restart_performed_iterations(const int num_iteration) {
  _file << LINE_PREFIX << indent_1 << "Performed Iterations: " << num_iteration
        << std::endl;
}
void UserFile::restart_best_iteration(const int best_iteration) {
  _file << LINE_PREFIX << indent_1 << "Best Iteration: " << best_iteration
        << std::endl;
}
void UserFile::restart_best_iterations_infos(
    const LogData &best_iteration_data) {
  const double overall_cost =
      best_iteration_data.subproblem_cost + best_iteration_data.invest_cost;
  _file << LINE_PREFIX << indent_1 << "Best Iteration Infos: " << std::endl
        << LINE_PREFIX << indent_1 << std::endl;
  log_master_solving_duration(best_iteration_data.master_time);
  _file << LINE_PREFIX << indent_1
        << " Overall cost = " << commons::create_str_million_euros(overall_cost)
        << " Me" << std::endl;
  log_iteration_candidates(best_iteration_data);
}

}  // namespace logger
}  // namespace xpansion
