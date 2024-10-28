//
// Created by s20217 on 20/04/2021.
//

#ifndef ANTARESXPANSION_USER_H
#define ANTARESXPANSION_USER_H

#include <ostream>

#include "antares-xpansion/xpansion_interfaces/ILogger.h"

namespace xpansion {
namespace logger {

class User : public ILogger {
 public:
  User(std::ostream &stream);

  void display_message(const std::string &str) override;
  void display_message(const std::string &str, LogUtils::LOGLEVEL level,
                       const std::string &context) override;

  virtual void PrintIterationSeparatorBegin() override;
  virtual void PrintIterationSeparatorEnd() override;

  void log_at_initialization(const int it_number) override;

  void log_iteration_candidates(const LogData &d) override;

  void log_master_solving_duration(double durationInSeconds) override;

  void LogSubproblemsSolvingWalltime(double durationInSeconds) override;

  void LogSubproblemsSolvingCumulativeCpuTime(
      double durationInSeconds) override;

  void log_at_iteration_end(const LogData &d) override;

  void log_at_ending(const LogData &d) override;

  void log_total_duration(double durationInSeconds) override;

  void log_stop_criterion_reached(
      const StoppingCriterion stopping_criterion) override;

  void display_restart_message() override;
  void restart_elapsed_time(const double elapsed_time) override;
  void number_of_iterations_before_restart(const int num_iterations) override;
  void restart_best_iteration(const int best_iterations) override;
  void restart_best_iterations_infos(
      const LogData &best_iterations_data) override;
  void LogAtInitialRelaxation() override;
  void LogAtSwitchToInteger() override;
  void cumulative_number_of_sub_problem_solved(int number) override;

 private:
  std::ostream &_stream;
};

}  // namespace logger
}  // namespace xpansion

#endif  // ANTARESXPANSION_USER_H
