//
// Created by jmkerloch on 19/04/2021.
//

#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <map>
#include <vector>
#include <memory>
#include <string>

typedef std::map<std::string, double> LogPoint;

struct LogData {
    double lb;
    double best_ub;
    int it;
    double slave_cost;
    double invest_cost;
    LogPoint x0;
    LogPoint min_invest;
    LogPoint max_invest;
    double optimal_gap;
};

class ILogger {

public:
    virtual ~ILogger()= default;
    virtual void log_at_initialization(const LogData& d) = 0;
    virtual void log_at_iteration_start      (const LogData& d) = 0;
    virtual void log_at_iteration_end      (const LogData& d) = 0;
    virtual void log_at_ending        (const LogData& d) = 0;

};
using Logger = std::shared_ptr<ILogger>;

#endif //ANTARESXPANSION_ILOGGER_H
