#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <ostream>
#include <tbb/task_arena.h>

#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/logger/FilteredLogger.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/**
 * \class
 * \brief a decorator class to log in separate files for separate threads, with a given verbosity
 * level
 */
class MultithreadTBBLogger: public ILogger
{
public:
    /// @brief constructor for the MultithreadTBBLogger class
    /// @param logFolder the folder where all log files will be
    /// @param logFileName the basename of all log files, which will be prefixed by the thread id
    /// @param nbThreads the number of desired threads
    /// @param verbosity the lowest verbosity desired
    explicit MultithreadTBBLogger(std::filesystem::path logFolder,
                                  std::filesystem::path logFileName,
                                  int nbThreads,
                                  std::optional<LogUtils::LOGLEVEL> verbosity = std::nullopt);
    ~MultithreadTBBLogger() = default;

    void display_message(const std::string& str) override;
    void display_message(const std::string& str,
                         LogUtils::LOGLEVEL level,
                         const std::string& context) override;

    virtual void PrintIterationSeparatorBegin() override;
    virtual void PrintIterationSeparatorEnd() override;

    void log_at_initialization(const int it_number) override;

    void log_iteration_candidates(const LogData& d) override;

    void log_master_solving_duration(double durationInSeconds) override;

    void LogSubproblemsSolvingWalltime(double durationInSeconds) override;

    void LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds) override;

    void log_at_iteration_end(const LogData& d) override;

    void log_at_ending(const LogData& d) override;

    void log_total_duration(double durationInSeconds) override;

    void log_stop_criterion_reached(const StoppingCriterion stopping_criterion) override;

    void display_restart_message() override;
    void restart_elapsed_time(const double elapsed_time) override;
    void number_of_iterations_before_restart(const int num_iterations) override;
    void restart_best_iteration(const int best_iterations) override;
    void restart_best_iterations_infos(const LogData& best_iterations_data) override;
    void LogAtInitialRelaxation() override;
    void LogAtSwitchToInteger() override;
    void cumulative_number_of_sub_problem_solved(int number) override;

    Logger operator[](int threadId);

private:
    std::map<int, Logger>
      _loggers;         /// the collection of all loggers, 0 being used for single thread logging
    int _nbThreads = 1; /// the maximum number of threads
    LogUtils::LOGLEVEL _verbosity = LogUtils::LOGLEVEL::INFO; /// the lowest desired verbosity
};
