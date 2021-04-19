//
// Created by jmkerloch on 19/04/2021.
//

#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <map>
#include <string>

typedef std::map<std::string, double> LogPoint;

struct LogData {
    double lb;
    double ub;
    double best_ub;
    int it;
    double slave_cost;
    double invest_cost;
    int best_it;
    LogPoint bestx;
    LogPoint x0;
    LogPoint min_invest;
    LogPoint max_invest;
};

class ILogger {

public:

    virtual void log_at_initialization(const LogData& d) = 0;
    virtual void log_at_iteration      (const LogData& d) = 0;
    virtual void log_at_ending        (const LogData& d) = 0;

};


#endif //ANTARESXPANSION_ILOGGER_H
