
#ifndef ANTARESXPANSION_LOGGERFACTORIES_H
#define ANTARESXPANSION_LOGGERFACTORIES_H

#include <filesystem>

#include "ILogger.h"
#include "SimulationOptions.h"
#include "logger/Master.h"
#include "logger/MathLogger.h"
#include "logger/UserFile.h"
Logger build_void_logger();

std::ostringstream start_message(const SimulationOptions &options,
                                 const std::string &benders_type);

class FileAndStdoutLoggerFactory {
 private:
  Logger logger;

 public:
  explicit FileAndStdoutLoggerFactory(
      const std::filesystem::path &report_file_path_string) {
    auto masterLogger = std::make_shared<xpansion::logger::Master>();
    auto user_file =
        std::make_shared<xpansion::logger::UserFile>(report_file_path_string);

    auto loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
    masterLogger->addLogger(user_file);
    masterLogger->addLogger(loggerUser);
    logger = masterLogger;
  }
  inline Logger get_logger() const { return logger; }
};

class MathLoggerFactory {
 private:
  MathLoggerDriver math_Logger_driver;
  MathLoggerFile math_logger_file_;
  MathLoggerOstream math_Logger_ostream_;

 public:
  explicit MathLoggerFactory(
      bool console_log, const std::filesystem::path &math_logs_file_path = "") {
    if (math_logs_file_path != "") {
      math_logger_file_ = MathLoggerFile(math_logs_file_path);
      math_Logger_driver.add_logger(&math_logger_file_);
    }
    if (console_log) {
      math_Logger_driver.add_logger(&math_Logger_ostream_);
    }
  }
  explicit MathLoggerFactory() = default;
  std::shared_ptr<MathLoggerDriver> get_logger() {
    return std::make_shared<MathLoggerDriver>(math_Logger_driver);
  }
};
#endif  // ANTARESXPANSION_LOGGERFACTORIES_H
