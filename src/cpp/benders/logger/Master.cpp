#include "antares-xpansion/benders/logger/Master.h"

namespace xpansion {
namespace logger {

void Master::display_message(const std::string &str) {
  for (auto logger : _loggers) {
    logger->display_message(str);
  }
}
void Master::display_message(const std::string &str, LogUtils::LOGLEVEL level,
                             const std::string &context) {
  for (auto logger : _loggers) {
    logger->display_message(str, level, context);
  }
}

void Master::log_at_initialization(const int it_number) {
  for (auto logger : _loggers) {
    logger->log_at_initialization(it_number);
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

void Master::LogSubproblemsSolvingWalltime(double durationInSeconds) {
  for (auto logger : _loggers) {
    logger->LogSubproblemsSolvingWalltime(durationInSeconds);
  }
}

void Master::LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds) {
  for (auto logger : _loggers) {
    logger->LogSubproblemsSolvingCumulativeCpuTime(durationInSeconds);
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

void Master::log_stop_criterion_reached(
    const StoppingCriterion stopping_criterion) {
  for (auto logger : _loggers) {
    logger->log_stop_criterion_reached(stopping_criterion);
  }
}
void Master::display_restart_message() {
  for (auto logger : _loggers) {
    logger->display_restart_message();
  }
}
void Master::restart_elapsed_time(const double elapsed_time) {
  for (auto logger : _loggers) {
    logger->restart_elapsed_time(elapsed_time);
  }
}
void Master::number_of_iterations_before_restart(const int num_iteration) {
  for (auto logger : _loggers) {
    logger->number_of_iterations_before_restart(num_iteration);
  }
}
void Master::restart_best_iteration(const int best_iteration) {
  for (auto logger : _loggers) {
    logger->restart_best_iteration(best_iteration);
  }
}
void Master::restart_best_iterations_infos(const LogData &best_iteration_data) {
  for (auto logger : _loggers) {
    logger->restart_best_iterations_infos(best_iteration_data);
  }
}

void Master::LogAtInitialRelaxation() {
  for (auto logger : _loggers) {
    logger->LogAtInitialRelaxation();
  }
}

void Master::LogAtSwitchToInteger() {
  for (auto logger : _loggers) {
    logger->LogAtSwitchToInteger();
  }
}

void Master::cumulative_number_of_sub_problem_solved(int number) {
  for (auto logger : _loggers) {
    logger->cumulative_number_of_sub_problem_solved(number);
  }
}

void Master::PrintIterationSeparatorBegin() {
  for (auto logger : _loggers) {
    logger->PrintIterationSeparatorBegin();
  }
}
void Master::PrintIterationSeparatorEnd() {
  for (auto logger : _loggers) {
    logger->PrintIterationSeparatorEnd();
  }
}

}  // namespace logger
}  // namespace xpansion
