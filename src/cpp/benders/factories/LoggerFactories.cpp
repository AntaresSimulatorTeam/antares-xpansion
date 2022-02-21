
#include "LoggerFactories.h"
#include "logger/Master.h"
#include "logger/UserFile.h"

Logger build_void_logger() {
  Logger logger = std::make_shared<xpansion::logger::Master>();
  return logger;
}

Logger build_stdout_and_file_logger(const std::string &report_file_path_string,
                                    FILE *&fp) {
  auto masterLogger = std::make_shared<xpansion::logger::Master>();
  auto user_file =
      std::make_shared<xpansion::logger::UserFile>(report_file_path_string);
  std::cout << "fp " << fp << "\n";
  //   Logger loggerFile =
  //   std::make_shared<xpansion::logger::UserFile>(user_file);
  fp = user_file->get_file_handler()._fp;

  Logger loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
  masterLogger->addLogger(user_file);
  masterLogger->addLogger(loggerUser);
  Logger logger = masterLogger;
  return logger;
}

std::ostringstream start_message(const SimulationOptions &options,
                                 const std::string &benders_type) {
  std::ostringstream oss_l;
  oss_l << "starting Benders " << benders_type << std::endl;
  options.print(oss_l);
  return oss_l;
}
