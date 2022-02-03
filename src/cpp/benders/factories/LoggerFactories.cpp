
#include "LoggerFactories.h"
#include "logger/Master.h"
#include "logger/UserFile.h"

Logger build_void_logger() {
  Logger logger = std::make_shared<xpansion::logger::Master>();
  return logger;
}

Logger
build_stdout_and_file_logger(const std::string &report_file_path_string) {
  auto masterLogger = std::make_shared<xpansion::logger::Master>();
  Logger loggerFile =
      std::make_shared<xpansion::logger::UserFile>(report_file_path_string);
  Logger loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
  masterLogger->addLogger(loggerFile);
  masterLogger->addLogger(loggerUser);
  Logger logger = masterLogger;
  return logger;
}

std::ostringstream start_message(const BendersOptions &options,
                                 const std::string &benders_type) {
  std::ostringstream oss_l;
  oss_l << "starting Benders " << benders_type << std::endl;
  options.print(oss_l);
  return oss_l;
}