#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>

#include "logger/UserFile.h"

namespace xpansion {
namespace logger {

UserFile::UserFile(const std::string &filename) {
  _file.open(filename);
  if (_file.fail()) {
    std::cerr << "Invalid file name passed as parameter" << std::endl;
  }
  _userLog = std::unique_ptr<User>(new User(_file));
}

UserFile::~UserFile() { _file.close(); }

void UserFile::display_message(const std::string &str) {
  _userLog->display_message(str);
}

void UserFile::log_at_initialization(const LogData &d) {
  _userLog->log_at_initialization(d);
}

void UserFile::log_iteration_candidates(const LogData &d) {
  _userLog->log_iteration_candidates(d);
}

void UserFile::log_master_solving_duration(double durationInSeconds) {
  _userLog->log_master_solving_duration(durationInSeconds);
}

void UserFile::log_subproblems_solving_duration(double durationInSeconds) {
  _userLog->log_subproblems_solving_duration(durationInSeconds);
}

void UserFile::log_at_iteration_end(const LogData &d) {
  _userLog->log_at_iteration_end(d);
}

void UserFile::log_at_ending(const LogData &d) { _userLog->log_at_ending(d); }

void UserFile::log_total_duration(double durationInSeconds) {
  _userLog->log_total_duration(durationInSeconds);
}

void UserFile::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  _userLog->log_stop_criterion_reached(stopping_criterion);
}

} // namespace logger
} // namespace xpansion
