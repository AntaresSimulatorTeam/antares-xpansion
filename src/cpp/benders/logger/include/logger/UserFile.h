#ifndef ANTARESXPANSION_CONSOLE_H
#define ANTARESXPANSION_CONSOLE_H

#include <fstream>
#include <ostream>

#include "core/ILogger.h"
#include "logger/User.h"
#include <cstdio>
namespace xpansion {
namespace logger {

class FileHandler {
public:
  FileHandler(const std::string &filename) : _filename(filename) {
    _fp = fopen(_filename.c_str(), "a+");
    if (_fp == NULL) {
      std::cerr << "Invalid file name passed as parameter" << std::endl;
    }
    setvbuf(_fp, NULL, _IONBF, 0);
  }
  FileHandler(const FileHandler &copy) : FileHandler(copy._filename) {}
  ~FileHandler() { fclose(_fp); }
  template <typename T> FileHandler &operator<<(const T &t) {
    std::ostringstream _s;
    _s << t;
    auto str = _s.str();
    const char *msg = str.c_str();
    fprintf(_fp, "%s", msg);
    return *this;
  }

  // for templated function like std::endl
  FileHandler &operator<<(std::ostream &(*t)(std::ostream &)) {
    std::ostringstream _s;
    t(_s);

    auto str = _s.str();
    return operator<<(str);
  }

public:
  std::string _filename;
  FILE *_fp;
};
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

  FileHandler &get_file_handler();

private:
  std::ofstream _file;
  std::string _filename;
  std::unique_ptr<User> _userLog;
  FileHandler _file_handler;
};

} // namespace logger
} // namespace xpansion

#endif // ANTARESXPANSION_CONSOLE_H
