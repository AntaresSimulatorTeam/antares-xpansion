#include "antares-xpansion/benders/benders_core/LastIterationPrinter.h"

#include <iostream>

LastIterationPrinter::LastIterationPrinter(Logger &logger,
                                           const LogData &best_iteration,
                                           const LogData &last_iteration)
    : logger_(logger),
      best_iteration_data_(best_iteration),
      last_iteration_data_(last_iteration) {}
void LastIterationPrinter::Print() const {
  logger_->display_restart_message();
  logger_->restart_elapsed_time(last_iteration_data_.benders_elapsed_time);
  logger_->number_of_iterations_before_restart(last_iteration_data_.it);
  logger_->restart_best_iteration(last_iteration_data_.best_it);
  logger_->restart_best_iterations_infos(best_iteration_data_);
}
