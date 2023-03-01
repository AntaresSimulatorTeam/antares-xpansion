
#include "LoggerFactories.h"

#include "logger/Master.h"
#include "logger/UserFile.h"

Logger build_void_logger() {
  Logger logger = std::make_shared<xpansion::logger::Master>();
  return logger;
}

std::ostringstream start_message(const SimulationOptions &options,
                                 const std::string &benders_type) {
  std::ostringstream oss_l;
  oss_l << "starting " << benders_type << std::endl;
  options.print(oss_l);
  return oss_l;
}
