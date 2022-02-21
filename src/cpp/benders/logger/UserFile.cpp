#include "CandidateLog.h"
#include "Commons.h"
#include "IterationResultLog.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include "logger/UserFile.h"

namespace xpansion {
namespace logger {

UserFile::UserFile(const std::string &filename)
    : _filename(filename), _file_handler(filename) {}

UserFile::UserFile(const UserFile &copy)
    : _filename(copy._filename), _file_handler(copy._file_handler) {
  // _file_handler = copy._file_handler;
  std::cout << " _file_handler._fp " << copy._file_handler._fp << "\n";
  std::cout << " _file_handler._fp " << _file_handler._fp << "\n";
}
// _userLog = std::unique_ptr<User>(new User(_file));

UserFile::~UserFile() {}

void UserFile::display_message(const std::string &str) {
  _file_handler << str << std::endl;
}

void UserFile::log_at_initialization(const LogData &d) {
  _file_handler << "ITERATION " << d.it << ":" << std::endl;
}

void UserFile::log_iteration_candidates(const LogData &d) {
  _file_handler << CandidateLog().log_iteration_candidates(d);
}

void UserFile::log_master_solving_duration(double durationInSeconds) {
  _file_handler << indent_1 << "Master solved in " << durationInSeconds << " s"
                << std::endl;
}

void UserFile::log_subproblems_solving_duration(double durationInSeconds) {
  _file_handler << indent_1 << "Subproblems solved in " << durationInSeconds
                << " s" << std::endl;
}

void UserFile::log_at_iteration_end(const LogData &d) {
  _file_handler << IterationResultLog().Log_at_iteration_end(d);
}

void UserFile::log_at_ending(const LogData &d) {
  const double overall_cost = d.slave_cost + d.invest_cost;
  _file_handler << indent_1 << "Best solution = it " << d.best_it << std::endl;
  _file_handler << indent_1 << " Overall cost = "
                << commons::create_str_million_euros(overall_cost) << " Me"
                << std::endl;
}

void UserFile::log_total_duration(double durationInSeconds) {
  _file_handler << "Problem ran in " << durationInSeconds << " s" << std::endl;
}

void UserFile::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  _file_handler << "--- Run completed: " << criterion_to_str(stopping_criterion)
                << " reached" << std::endl;
}
FileHandler &UserFile::get_file_handler() { return _file_handler; }

} // namespace logger
} // namespace xpansion
