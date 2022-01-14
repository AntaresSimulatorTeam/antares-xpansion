#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <map>
#include <vector>
#include <memory>
#include <string>

typedef std::map<std::string, double> LogPoint;

enum class StoppingCriterion {empty, timelimit, relative_gap, absolute_gap, max_iteration};

struct LogData {
    double lb;
    double best_ub;
    int it;
    int best_it;
    double slave_cost;
    double invest_cost;
    LogPoint x0;
    LogPoint min_invest;
    LogPoint max_invest;
    double optimality_gap;
    double relative_gap;
    double max_iterations;
};

class ILogger {

public:
    virtual ~ILogger()= default;
    
    virtual void display_message(const std::string& str) = 0;
    virtual void log_at_initialization(const LogData& d) = 0;
    virtual void log_iteration_candidates      (const LogData& d) = 0;
    virtual void log_master_solving_duration(double durationInSeconds) = 0;
    virtual void log_subproblems_solving_duration(double durationInSeconds) = 0;
    virtual void log_at_iteration_end      (const LogData& d) = 0;
    virtual void log_at_ending        (const LogData& d) = 0;
    virtual void log_total_duration(double durationInSeconds) = 0;
    virtual void log_stop_criterion_reached(const StoppingCriterion stopping_criterion) = 0 ;
};
using Logger = std::shared_ptr<ILogger>;

#endif //ANTARESXPANSION_ILOGGER_H
