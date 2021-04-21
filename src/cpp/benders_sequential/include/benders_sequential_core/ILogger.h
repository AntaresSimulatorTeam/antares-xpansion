//
// Created by jmkerloch on 19/04/2021.
//

#ifndef ANTARESXPANSION_ILOGGER_H
#define ANTARESXPANSION_ILOGGER_H

#include <map>
#include <vector>
#include <string>

typedef std::map<std::string, double> LogPoint;

struct LogData {
    int nbasis;
    double timer_slaves;
    double timer_master;
    double lb;
    double ub;
    double best_ub;
    int maxsimplexiter;
    int minsimplexiter;
    int deletedcut;
    int it;
    bool stop;
    double alpha;
    std::vector<double> alpha_i;
    double slave_cost;
    double invest_cost;
    int best_it;
    LogPoint bestx;
    LogPoint x0;
    LogPoint min_invest;
    LogPoint max_invest;
    int nslaves;
    double dnslaves;
    int master_status;
    int nrandom;
    double optimal_gap;
};

class ILogger {

public:

    virtual void log_at_initialization(const LogData& d) = 0;
    virtual void log_at_iteration_start      (const LogData& d) = 0;
    virtual void log_at_iteration_end      (const LogData& d) = 0;
    virtual void log_at_ending        (const LogData& d) = 0;

};


#endif //ANTARESXPANSION_ILOGGER_H
