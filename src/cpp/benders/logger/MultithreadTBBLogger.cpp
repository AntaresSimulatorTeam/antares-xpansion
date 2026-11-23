#pragma once

#include "antares-xpansion/benders/logger/MultithreadTBBLogger.h"

MultithreadTBBLogger::MultithreadTBBLogger(std::filesystem::path logFolder,
                                           std::filesystem::path logFileName,
                                           int nbThreads,
                                           std::optional<LogUtils::LOGLEVEL> verbosity):
    _nbThreads(nbThreads)
{
    _verbosity = verbosity.value_or(LogUtils::LOGLEVEL::INFO);
    // defaulting to FilteredLogger loggers for threads
    // assuming thread ids range from 0 to nbThreads - 1;
    // also assuming that nbThreads is not greater than the maximum hardware concurrency
    // otherwise will create empty log files
    for (int threadId = 0; threadId < _nbThreads; ++threadId)
    {
        auto loggerFactory = FileAndStdoutLoggerFactory(
          logFolder / (std::to_string(threadId) + "_" + logFileName.string()),
          false);
        _loggers.emplace(threadId,
                         std::make_shared<FilteredLogger>(loggerFactory.get_logger(), _verbosity));
    }

    if (!std::filesystem::exists(logFolder))
    {
        _loggers.at(0)->display_message(
          "Folder for multithread log files did not exist, it is created at " + logFolder.string());
        std::filesystem::create_directories(logFolder);
    }
}

Logger MultithreadTBBLogger::operator[](int threadId)
{
    return _loggers.at(threadId);
}

void MultithreadTBBLogger::display_message(const std::string& str)
{
    // this is the TBB-dependent part, which could be improved with an additional layer of
    // abstraction
    int threadId = tbb::this_task_arena::current_thread_index();
    if (threadId != tbb::task_arena::not_initialized && _loggers.count(threadId))
    {
        _loggers.at(threadId)->display_message(str);
    }
    else
    {
        _loggers.at(0)->display_message(str);
    }
}

void MultithreadTBBLogger::display_message(const std::string& str,
                                           LogUtils::LOGLEVEL level,
                                           const std::string& context)
{
    // this is the TBB-dependent part, which could be improved with an additional layer of
    // abstraction
    int threadId = tbb::this_task_arena::current_thread_index();
    if (threadId != tbb::task_arena::not_initialized && _loggers.count(threadId))
    {
        _loggers.at(threadId)->display_message(str, level, context);
    }
    else
    {
        _loggers.at(0)->display_message(str, level, context);
    }
}

void MultithreadTBBLogger::log_at_initialization(const int it_number)
{
    _loggers.at(0)->log_at_initialization(it_number);
}

void MultithreadTBBLogger::log_iteration_candidates(const LogData& d)
{
    _loggers.at(0)->log_iteration_candidates(d);
}

void MultithreadTBBLogger::log_master_solving_duration(double durationInSeconds)
{
    _loggers.at(0)->log_master_solving_duration(durationInSeconds);
}

void MultithreadTBBLogger::LogSubproblemsSolvingWalltime(double durationInSeconds)
{
    _loggers.at(0)->LogSubproblemsSolvingWalltime(durationInSeconds);
}

void MultithreadTBBLogger::LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds)
{
    _loggers.at(0)->LogSubproblemsSolvingCumulativeCpuTime(durationInSeconds);
}

void MultithreadTBBLogger::log_at_iteration_end(const LogData& d)
{
    _loggers.at(0)->log_at_iteration_end(d);
}

void MultithreadTBBLogger::log_at_ending(const LogData& d)
{
    _loggers.at(0)->log_at_ending(d);
}

void MultithreadTBBLogger::log_total_duration(double durationInSeconds)
{
    _loggers.at(0)->log_total_duration(durationInSeconds);
}

void MultithreadTBBLogger::log_stop_criterion_reached(const StoppingCriterion stopping_criterion)
{
    _loggers.at(0)->log_stop_criterion_reached(stopping_criterion);
}

void MultithreadTBBLogger::display_restart_message()
{
    _loggers.at(0)->display_restart_message();
}

void MultithreadTBBLogger::restart_elapsed_time(const double elapsed_time)
{
    _loggers.at(0)->restart_elapsed_time(elapsed_time);
}

void MultithreadTBBLogger::number_of_iterations_before_restart(const int num_iteration)
{
    _loggers.at(0)->number_of_iterations_before_restart(num_iteration);
}

void MultithreadTBBLogger::restart_best_iteration(const int best_iteration)
{
    _loggers.at(0)->restart_best_iteration(best_iteration);
}

void MultithreadTBBLogger::restart_best_iterations_infos(const LogData& best_iteration_data)
{
    _loggers.at(0)->restart_best_iterations_infos(best_iteration_data);
}

void MultithreadTBBLogger::LogAtInitialRelaxation()
{
    _loggers.at(0)->LogAtInitialRelaxation();
}

void MultithreadTBBLogger::LogAtSwitchToInteger()
{
    _loggers.at(0)->LogAtSwitchToInteger();
}

void MultithreadTBBLogger::cumulative_number_of_sub_problem_solved(int number)
{
    _loggers.at(0)->cumulative_number_of_sub_problem_solved(number);
}

void MultithreadTBBLogger::PrintIterationSeparatorBegin()
{
    _loggers.at(0)->PrintIterationSeparatorBegin();
}

void MultithreadTBBLogger::PrintIterationSeparatorEnd()
{
    _loggers.at(0)->PrintIterationSeparatorEnd();
}
