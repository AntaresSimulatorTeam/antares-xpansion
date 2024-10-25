#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

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
  int cumulative_number_of_subproblem_resolved;
};

/**
 * \interface ILoggerXpansion
 * \brief Xpansion Unique log Interface
 */
struct ILoggerXpansion {
  /**
   * \brief pure virtual method to display a std::string message
   * \param str the message to be displayed
   */
  virtual void display_message(const std::string &str) = 0;

  /**
   * display the underlying std::string in std::ostringstream
   * \param str the message to be displayed
   */
  void display_message(const std::ostringstream &msg) {
    display_message(msg.str());
  }

  virtual void display_message(const std::string &msg, LogUtils::LOGLEVEL level,
                               const std::string &context) = 0;

  virtual void PrintIterationSeparatorBegin() = 0;
  virtual void PrintIterationSeparatorEnd() = 0;
  virtual ~ILoggerXpansion() = default;
};

/**
 * useful for multi-proc run
 */
struct EmptyLogger : public ILoggerXpansion {
  void display_message(const std::string &str) override {}
  void display_message(const std::string &str, LogUtils::LOGLEVEL level,
                       const std::string &context) override {}

  void PrintIterationSeparatorBegin() override {};
  void PrintIterationSeparatorEnd() override {};
  virtual ~EmptyLogger() {}

};

/**
 * this \class act like a log aggregator
 */
struct BendersLoggerBase : public ILoggerXpansion {
  void display_message(const std::string &str) override {
    for (auto logger : loggers) {
      logger->display_message(str);
    }
  }
  void AddLogger(std::shared_ptr<ILoggerXpansion> logger) {
    loggers.push_back(logger);
  }

  virtual void PrintIterationSeparatorBegin() override {
    for (auto logger : loggers) {
      logger->PrintIterationSeparatorBegin();
    }
  }

  virtual void PrintIterationSeparatorEnd() override {
    for (auto logger : loggers) {
      logger->PrintIterationSeparatorEnd();
    }
  }
  virtual void display_message(const std::string &msg, LogUtils::LOGLEVEL level,
                               const std::string &context) override {
    for (auto logger : loggers) {
      logger->display_message(msg, level, context);
    }
  }

 private:
  std::vector<std::shared_ptr<ILoggerXpansion>> loggers;
};

/**
 * \interface
 * \brief abstract class for operational logs
 */
class ILogger : public ILoggerXpansion {
 public:
  virtual ~ILogger() = default;

  virtual void display_message(const std::string &str) = 0;
  virtual void display_message(const std::string &str, LogUtils::LOGLEVEL level,
                               const std::string &context) = 0;
  virtual void PrintIterationSeparatorBegin() = 0;
  virtual void PrintIterationSeparatorEnd() = 0;
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
  virtual void cumulative_number_of_sub_problem_solved(int number) = 0;
  const std::string CONTEXT = "Benders";
};

using Logger = std::shared_ptr<ILogger>;

#endif  // ANTARESXPANSION_ILOGGER_H
