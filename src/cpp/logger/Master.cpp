//
// Created by jmkerloch on 19/04/2021.
//

#include "logger/Master.h"

namespace xpansion{
namespace logger {

    Master::Master() {}

    Master::~Master() {

    }

    void Master::log_at_initialization(const LogData &d) {
        for (auto logger : _loggers) {
            logger->log_at_initialization(d);
        }
    }

    void Master::log_at_iteration_start(const LogData &d) {
        for (auto logger : _loggers) {
            logger->log_at_iteration_start(d);
        }
    }

    void Master::log_at_iteration_end(const LogData &d) {
        for (auto logger : _loggers) {
            logger->log_at_iteration_end(d);
        }
    }

    void Master::log_at_ending(const LogData &d) {
        for (auto logger : _loggers) {
            logger->log_at_ending(d);
        }
    }

} // namespace logger
} // namespace xpansion

