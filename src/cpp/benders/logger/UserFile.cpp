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

UserFile::UserFile(const std::string &filename) {
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

void UserFile::log_at_initialization(const LogData &d) {
  _file << LINE_PREFIX << "ITERATION " << d.it << ":" << std::endl;
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
  const double overall_cost = d.slave_cost + d.invest_cost;
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

}  // namespace logger
}  // namespace xpansion
