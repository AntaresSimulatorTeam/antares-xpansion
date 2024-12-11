
#include "antares-xpansion/benders/factories/LoggerFactories.h"

#include "antares-xpansion/benders/logger/Master.h"
#include "antares-xpansion/benders/logger/UserFile.h"

Logger build_void_logger() {
  Logger logger = std::make_shared<xpansion::logger::Master>();
  return logger;
}
