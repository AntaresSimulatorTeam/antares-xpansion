
#ifndef ANTARESXPANSION_LOGGERFACTORIES_H
#define ANTARESXPANSION_LOGGERFACTORIES_H

#include <filesystem>

#include "BendersFactory.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/logger/Master.h"
#include "antares-xpansion/benders/logger/MathLogger.h"
#include "antares-xpansion/benders/logger/UserFile.h"
Logger build_void_logger();

class FileAndStdoutLoggerFactory {
 private:
  Logger logger;

 public:
  explicit FileAndStdoutLoggerFactory(
      const std::filesystem::path &report_file_path_string,
      bool expert_log_at_console) {
    auto masterLogger = std::make_shared<xpansion::logger::Master>();
    auto user_file =
        std::make_shared<xpansion::logger::UserFile>(report_file_path_string);
    masterLogger->addLogger(user_file);

    if (!expert_log_at_console) {
      auto loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
      masterLogger->addLogger(loggerUser);
    }

    logger = masterLogger;
  }
  inline Logger get_logger() const { return logger; }
};

class MathLoggerFactory {
 private:
  MathLoggerDriver math_logger_driver;

 public:
  explicit MathLoggerFactory(
      const BENDERSMETHOD &method, bool console_log,
      const std::filesystem::path &math_logs_file_path = "") {
    if (math_logs_file_path != "") {
      auto math_logger_file =
          std::make_shared<MathLoggerFile>(method, math_logs_file_path);
      math_logger_driver.add_logger(math_logger_file);
    }

      if (console_log) {
        auto math_logger_ostream = std::make_shared<MathLoggerOstream>(method);

        math_logger_driver.add_logger(math_logger_ostream);
      }
  }
  explicit MathLoggerFactory() = default;
  std::shared_ptr<MathLoggerDriver> get_logger() {
    return std::make_shared<MathLoggerDriver>(math_logger_driver);
  }
  static std::shared_ptr<MathLoggerDriver> get_void_logger() {
    return std::make_shared<MathLoggerDriver>();
  }
  };
#endif  // ANTARESXPANSION_LOGGERFACTORIES_H
