#ifndef ANTARESXPANSION_CONSOLE_H
#define ANTARESXPANSION_CONSOLE_H

#include <cstdio>
#include <fstream>
#include <ostream>

#include "core/ILogger.h"
#include "logger/User.h"
namespace xpansion {
namespace logger {

class UserFile : public ILogger {
 public:
  explicit UserFile(const std::string &filename);
  explicit UserFile(const UserFile &copy);
  ~UserFile();

  void display_message(const std::string &str) override;

  void log_at_initialization(const LogData &d) override;

  void log_iteration_candidates(const LogData &d) override;

  void log_master_solving_duration(double durationInSeconds) override;

  void log_subproblems_solving_duration(double durationInSeconds) override;

  void log_at_iteration_end(const LogData &d) override;

  void log_at_ending(const LogData &d) override;

  void log_total_duration(double durationInSeconds) override;

  void log_stop_criterion_reached(
      const StoppingCriterion stopping_criterion) override;

  const std::string LINE_PREFIX = "<<BENDERS>> ";

 private:
  std::ofstream _file;
  std::string _filename;
};

}  // namespace logger
}  // namespace xpansion

#endif  // ANTARESXPANSION_CONSOLE_H
