#pragma once

#include "antares-xpansion/benders/logger/MultithreadLogger.h"

MultithreadLogger::MultithreadLogger(Logger logger,
                                     std::filesystem::path logFolder,
                                     std::filesystem::path logFileName,
                                     int nbThreads):
    _logger(logger),
    _nbThreads(nbThreads)
{
    if (!std::filesystem::exists(logFolder))
    {
        _logger->display_message("Folder for multithread log files did not exist, it is created at "
                                 + logFolder.string());
        std::filesystem::create_directories(logFolder);
    }
    // defaulting to FileAndStdout loggers for threads
    // assuming threads going from 0 to nbThreads - 1
    for (int threadId = 0; threadId < _nbThreads; ++threadId)
    {
        auto loggerFactory = FileAndStdoutLoggerFactory(
          logFolder / (std::to_string(threadId) + "_" + logFileName.string()),
          false);
        _loggers.emplace(threadId, loggerFactory.get_logger());
    }
}

Logger MultithreadLogger::operator[](int threadId)
{
    return _loggers.at(threadId);
}

void MultithreadLogger::display_message(const std::string& str)
{
    int threadId = tbb::this_task_arena::current_thread_index();
    if (_loggers.count(threadId))
    {
        _loggers.at(threadId)->display_message(str);
    }
    else
    {
        _logger->display_message(str);
    }
}

void MultithreadLogger::display_message(const std::string& str,
                                        LogUtils::LOGLEVEL level,
                                        const std::string& context)
{
    int threadId = tbb::this_task_arena::current_thread_index();
    if (_loggers.count(threadId))
    {
        _loggers.at(threadId)->display_message(str, level, context);
    }
    else
    {
        _logger->display_message(str, level, context);
    }
}

void MultithreadLogger::log_at_initialization(const int it_number)
{
    _logger->log_at_initialization(it_number);
}

void MultithreadLogger::log_iteration_candidates(const LogData& d)
{
    _logger->log_iteration_candidates(d);
}

void MultithreadLogger::log_master_solving_duration(double durationInSeconds)
{
    _logger->log_master_solving_duration(durationInSeconds);
}

void MultithreadLogger::LogSubproblemsSolvingWalltime(double durationInSeconds)
{
    _logger->LogSubproblemsSolvingWalltime(durationInSeconds);
}

void MultithreadLogger::LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds)
{
    _logger->LogSubproblemsSolvingCumulativeCpuTime(durationInSeconds);
}

void MultithreadLogger::log_at_iteration_end(const LogData& d)
{
    _logger->log_at_iteration_end(d);
}

void MultithreadLogger::log_at_ending(const LogData& d)
{
    _logger->log_at_ending(d);
}

void MultithreadLogger::log_total_duration(double durationInSeconds)
{
    _logger->log_total_duration(durationInSeconds);
}

void MultithreadLogger::log_stop_criterion_reached(const StoppingCriterion stopping_criterion)
{
    _logger->log_stop_criterion_reached(stopping_criterion);
}

void MultithreadLogger::display_restart_message()
{
    _logger->display_restart_message();
}

void MultithreadLogger::restart_elapsed_time(const double elapsed_time)
{
    _logger->restart_elapsed_time(elapsed_time);
}

void MultithreadLogger::number_of_iterations_before_restart(const int num_iteration)
{
    _logger->number_of_iterations_before_restart(num_iteration);
}

void MultithreadLogger::restart_best_iteration(const int best_iteration)
{
    _logger->restart_best_iteration(best_iteration);
}

void MultithreadLogger::restart_best_iterations_infos(const LogData& best_iteration_data)
{
    _logger->restart_best_iterations_infos(best_iteration_data);
}

void MultithreadLogger::LogAtInitialRelaxation()
{
    _logger->LogAtInitialRelaxation();
}

void MultithreadLogger::LogAtSwitchToInteger()
{
    _logger->LogAtSwitchToInteger();
}

void MultithreadLogger::cumulative_number_of_sub_problem_solved(int number)
{
    _logger->cumulative_number_of_sub_problem_solved(number);
}

void MultithreadLogger::PrintIterationSeparatorBegin()
{
    _logger->PrintIterationSeparatorBegin();
}

void MultithreadLogger::PrintIterationSeparatorEnd()
{
    _logger->PrintIterationSeparatorEnd();
}
