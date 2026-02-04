#pragma once

#include "antares-xpansion/benders/logger/FilteredLogger.h"

FilteredLogger::FilteredLogger(Logger logger, LogUtils::LOGLEVEL minimumLogLevel):
    _logger(logger),
    _minimumLogLevel(minimumLogLevel)
{
}

void FilteredLogger::display_message(const std::string& str)
{
    if (_minimumLogLevel == LogUtils::LOGLEVEL::NONE)
    {
        return;
    }
    if (_minimumLogLevel <= LogUtils::LOGLEVEL::INFO)
    {
        _logger->display_message(str);
    }
}

void FilteredLogger::display_message(const std::string& str,
                                     LogUtils::LOGLEVEL level,
                                     const std::string& context)
{
    if (_minimumLogLevel == LogUtils::LOGLEVEL::NONE)
    {
        return;
    }
    if (_minimumLogLevel <= level)
    {
        _logger->display_message(str, level, context);
    }
}

void FilteredLogger::log_at_initialization(const int it_number)
{
    _logger->log_at_initialization(it_number);
}

void FilteredLogger::log_iteration_candidates(const LogData& d)
{
    _logger->log_iteration_candidates(d);
}

void FilteredLogger::log_master_solving_duration(double durationInSeconds)
{
    _logger->log_master_solving_duration(durationInSeconds);
}

void FilteredLogger::LogSubproblemsSolvingWalltime(double durationInSeconds)
{
    _logger->LogSubproblemsSolvingWalltime(durationInSeconds);
}

void FilteredLogger::LogSubproblemsSolvingCumulativeCpuTime(double durationInSeconds)
{
    _logger->LogSubproblemsSolvingCumulativeCpuTime(durationInSeconds);
}

void FilteredLogger::log_at_iteration_end(const LogData& d)
{
    _logger->log_at_iteration_end(d);
}

void FilteredLogger::log_at_ending(const LogData& d)
{
    _logger->log_at_ending(d);
}

void FilteredLogger::log_total_duration(double durationInSeconds)
{
    _logger->log_total_duration(durationInSeconds);
}

void FilteredLogger::log_stop_criterion_reached(const StoppingCriterion stopping_criterion)
{
    _logger->log_stop_criterion_reached(stopping_criterion);
}

void FilteredLogger::display_restart_message()
{
    _logger->display_restart_message();
}

void FilteredLogger::restart_elapsed_time(const double elapsed_time)
{
    _logger->restart_elapsed_time(elapsed_time);
}

void FilteredLogger::number_of_iterations_before_restart(const int num_iteration)
{
    _logger->number_of_iterations_before_restart(num_iteration);
}

void FilteredLogger::restart_best_iteration(const int best_iteration)
{
    _logger->restart_best_iteration(best_iteration);
}

void FilteredLogger::restart_best_iterations_infos(const LogData& best_iteration_data)
{
    _logger->restart_best_iterations_infos(best_iteration_data);
}

void FilteredLogger::LogAtInitialRelaxation()
{
    _logger->LogAtInitialRelaxation();
}

void FilteredLogger::LogAtSwitchToInteger()
{
    _logger->LogAtSwitchToInteger();
}

void FilteredLogger::cumulative_number_of_sub_problem_solved(int number)
{
    _logger->cumulative_number_of_sub_problem_solved(number);
}

void FilteredLogger::PrintIterationSeparatorBegin()
{
    _logger->PrintIterationSeparatorBegin();
}

void FilteredLogger::PrintIterationSeparatorEnd()
{
    _logger->PrintIterationSeparatorEnd();
}
