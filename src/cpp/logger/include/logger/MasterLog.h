//
// Created by jmkerloch on 19/04/2021.
//

#ifndef ANTARESXPANSION_MASTERLOG_H
#define ANTARESXPANSION_MASTERLOG_H

#include "benders_sequential_core/ILogger.h"

class MasterLog : public ILogger {

public:

    MasterLog();
    virtual ~MasterLog();

    void log_at_initialization(const LogData &d) override;

    void log_at_iteration(const LogData &d) override;

    void log_at_ending(const LogData &d) override;

};


#endif //ANTARESXPANSION_MASTERLOG_H
