#include "LastIterationPrinter.h"

#include <iostream>

LastIterationPrinter::LastIterationPrinter(Logger &logger,
                                           const LogData &best_iteration,
                                           const LogData &last_iteration)
    : _logger(logger),
      _best_iteration_data(best_iteration),
      _last_iteration_data(last_iteration) {}
void LastIterationPrinter::print() const {
  _logger->display_restart_message();
  _logger->restart_elapsed_time(_last_iteration_data.benders_elapsed_time);
  _logger->number_of_iterations_before_restart(_last_iteration_data.it);
  _logger->restart_best_iteration(_last_iteration_data.best_it);
  _logger->restart_best_iterations_infos(_best_iteration_data);
}
