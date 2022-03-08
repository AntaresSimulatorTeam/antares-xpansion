
#ifndef ANTARESXPANSION_LOGGERFACTORIES_H
#define ANTARESXPANSION_LOGGERFACTORIES_H

#include "SimulationOptions.h"
#include "core/ILogger.h"
#include "logger/Master.h"
#include "logger/UserFile.h"
Logger build_void_logger();

std::ostringstream start_message(const SimulationOptions &options,
                                 const std::string &benders_type);

class FileAndStdoutLoggerFactory {
 private:
  Logger logger;

 public:
  explicit FileAndStdoutLoggerFactory(
      const std::string &report_file_path_string) {
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
#endif  // ANTARESXPANSION_LOGGERFACTORIES_H
