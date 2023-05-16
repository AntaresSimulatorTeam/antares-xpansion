#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

typedef std::map<std::string, double> LogPoint;

enum class StoppingCriterion {
  empty,
  timelimit,
  relative_gap,
  absolute_gap,
  max_iteration
};
inline std::string criterion_to_str(
    const StoppingCriterion stopping_criterion) {
  std::string stop_crit("");
  switch (stopping_criterion) {
    case StoppingCriterion::absolute_gap:
      stop_crit = "absolute gap";
      break;

    case StoppingCriterion::relative_gap:
      stop_crit = "relative gap";
      break;

    case StoppingCriterion::max_iteration:
      stop_crit = "maximum iterations";
      break;

    case StoppingCriterion::timelimit:
      stop_crit = "timelimit";
      break;

    default:
      break;
  }
  return stop_crit;
}
struct LogData {
  double lb;
  double best_ub;
  double ub;
  int it;
  int best_it;
  double subproblem_cost;
  double invest_cost;
  LogPoint x_in;
  LogPoint x_out;
  LogPoint x_cut;
  LogPoint min_invest;
  LogPoint max_invest;
  double optimality_gap;
  double relative_gap;
  int max_iterations;
  double benders_elapsed_time;
  double master_time;
  double subproblem_time;
  bool operator==(const LogData &lhs) const {
    return lb == lhs.lb && best_ub == lhs.best_ub && ub == lhs.ub &&
           it == lhs.it && best_it == lhs.best_it &&
           subproblem_cost == lhs.subproblem_cost &&
           invest_cost == lhs.invest_cost && x_in == lhs.x_in &&
           x_out == lhs.x_out && x_cut == lhs.x_cut &&
           min_invest == lhs.min_invest && max_invest == lhs.max_invest &&
           optimality_gap == lhs.optimality_gap &&
           relative_gap == lhs.relative_gap &&
           max_iterations == lhs.max_iterations &&
           benders_elapsed_time == lhs.benders_elapsed_time &&
           master_time == lhs.master_time &&
           subproblem_time == lhs.subproblem_time;
  }
};
class ILogger {
 public:
  virtual ~ILogger() = default;

  virtual void display_message(const std::string &str) = 0;
  virtual void log_at_initialization(const int it_number) = 0;
  virtual void log_iteration_candidates(const LogData &d) = 0;
  virtual void log_master_solving_duration(double durationInSeconds) = 0;
  virtual void LogSubproblemsSolvingWalltime(double durationInSeconds) = 0;
  virtual void LogSubproblemsSolvingCumulativeCpuTime(
      double durationInSeconds) = 0;
  virtual void log_at_iteration_end(const LogData &d) = 0;
  virtual void log_at_ending(const LogData &d) = 0;
  virtual void log_total_duration(double durationInSeconds) = 0;
  virtual void log_stop_criterion_reached(
      const StoppingCriterion stopping_criterion) = 0;
  virtual void display_restart_message() = 0;
  virtual void restart_elapsed_time(const double elapsed_time) = 0;
  virtual void number_of_iterations_before_restart(
      const int num_iterations) = 0;
  virtual void restart_best_iteration(const int best_iterations) = 0;
  virtual void restart_best_iterations_infos(
      const LogData &best_iterations_data) = 0;
  virtual void LogAtInitialRelaxation() = 0;
  virtual void LogAtSwitchToInteger() = 0;
  virtual void number_of_sub_problem_resolved(int number) = 0;
  const std::string CONTEXT = "Benders";
};

using Logger = std::shared_ptr<ILogger>;

#endif  // ANTARESXPANSION_ILOGGER_H
