#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <time.h>

#include <filesystem>
#include <map>
#include <memory>
#include <sstream>
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
  LogPoint x0;
  LogPoint min_invest;
  LogPoint max_invest;
  double optimality_gap;
  double relative_gap;
  unsigned int max_iterations;
  double benders_elapsed_time;
  double master_time;
  bool operator==(const LogData &lhs) const {
    return lb == lhs.lb && best_ub == lhs.best_ub && ub == lhs.ub &&
           it == lhs.it && best_it == lhs.best_it &&
           subproblem_cost == lhs.subproblem_cost &&
           invest_cost == lhs.invest_cost && x0 == lhs.x0 &&
           min_invest == lhs.min_invest && max_invest == lhs.max_invest &&
           optimality_gap == lhs.optimality_gap &&
           relative_gap == lhs.relative_gap &&
           max_iterations == lhs.max_iterations &&
           benders_elapsed_time == lhs.benders_elapsed_time &&
           master_time == lhs.master_time;
  }
};
inline std::string format_time_str(const long int time_in_seconds) {
  std::time_t seconds(
      time_in_seconds);  // you have to convert your input_seconds into time_t
  std::tm p;

  // convert to broken down time
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  gmtime_s(&p, &seconds);
#else  // defined(__unix__) || (__APPLE__)
  gmtime_r(&seconds, &p);
#endif
  std::stringstream ss;
  const auto days = p.tm_yday;
  const auto hours = p.tm_hour;
  const auto mins = p.tm_min;
  const auto secs = p.tm_sec;
  if (days > 0) {
    ss << days << " days ";
  }
  if (hours > 0) {
    ss << hours << " hours ";
  }
  if (mins > 0) {
    ss << mins << " minutes ";
  }
  if (secs > 0) {
    ss << secs << " seconds ";
  }
  return ss.str();
}
class ILogger {
 public:
  virtual ~ILogger() = default;

  virtual void display_message(const std::string &str) = 0;
  virtual void log_at_initialization(const int it_number) = 0;
  virtual void log_iteration_candidates(const LogData &d) = 0;
  virtual void log_master_solving_duration(double durationInSeconds) = 0;
  virtual void log_subproblems_solving_duration(double durationInSeconds) = 0;
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
};

using Logger = std::shared_ptr<ILogger>;

#endif  // ANTARESXPANSION_ILOGGER_H
