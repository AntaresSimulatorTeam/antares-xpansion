//
// Created by jmkerloch on 19/04/2021.
//

#ifndef ANTARESXPANSION_MASTER_H
#define ANTARESXPANSION_MASTER_H

#include <iostream>
#include <fstream>
#include <list>
#include <memory>

#include "benders_sequential_core/ILogger.h"

class Master : public ILogger {

public:

    Master();
    virtual ~Master();

    void addLogger(const std::shared_ptr<ILogger>& logger) {_loggers.push_back(logger);}

    void log_at_initialization(const LogData &d) override;

    void log_at_iteration(const LogData &d) override;

    void log_at_ending(const LogData &d) override;

private:

    std::list<std::shared_ptr<ILogger>> _loggers;

};

#endif //ANTARESXPANSION_MASTER_H
