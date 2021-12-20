//
// Created by jmkerloch on 19/04/2021.
//

#ifndef ANTARESXPANSION_MASTER_H
#define ANTARESXPANSION_MASTER_H

#include <fstream>
#include <iostream>
#include <list>
#include <memory>

#include "core/ILogger.h"

namespace xpansion {
namespace logger {

class Master : public ILogger {

public:
  Master();
  virtual ~Master();

  void addLogger(const std::shared_ptr<ILogger> &logger) {
    _loggers.push_back(logger);
  }

  void display_message(const std::string &str) override;

  void log_at_initialization(const LogData &d) override;

  void log_iteration_candidates(const LogData &d) override;

  void log_master_solving_duration(double durationInSeconds) override;

  void log_subproblems_solving_duration(double durationInSeconds) override;

  void log_at_iteration_end(const LogData &d) override;

  void log_at_ending(const LogData &d) override;

  void log_total_duration(double durationInSeconds) override;

private:
  std::list<std::shared_ptr<ILogger>> _loggers;
};

} // namespace logger
} // namespace xpansion

#endif // ANTARESXPANSION_MASTER_H
