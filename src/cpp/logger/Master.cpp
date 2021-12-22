#include "logger/Master.h"

namespace xpansion {
namespace logger {

Master::Master() {}

Master::~Master() {}

void Master::display_message(const std::string &str) {
  for (auto logger : _loggers) {
    logger->display_message(str);
  }
}

void Master::log_at_initialization(const LogData &d) {
  for (auto logger : _loggers) {
    logger->log_at_initialization(d);
  }
}

void Master::log_iteration_candidates(const LogData &d) {
  for (auto logger : _loggers) {
    logger->log_iteration_candidates(d);
  }
}

void Master::log_master_solving_duration(double durationInSeconds) {
  for (auto logger : _loggers) {
    logger->log_master_solving_duration(durationInSeconds);
  }
}

void Master::log_subproblems_solving_duration(double durationInSeconds) {
  for (auto logger : _loggers) {
    logger->log_subproblems_solving_duration(durationInSeconds);
  }
}

void Master::log_at_iteration_end(const LogData &d) {
  for (auto logger : _loggers) {
    logger->log_at_iteration_end(d);
  }
}

void Master::log_at_ending(const LogData &d) {
  for (auto logger : _loggers) {
    logger->log_at_ending(d);
  }
}

void Master::log_total_duration(double durationInSeconds) {
  for (auto logger : _loggers) {
    logger->log_total_duration(durationInSeconds);
  }
}

} // namespace logger
} // namespace xpansion
