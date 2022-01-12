
#include "LoggerFactories.h"
#include "logger/UserFile.h"
#include "logger/Master.h"

Logger build_void_logger() {
    Logger logger = std::make_shared<xpansion::logger::Master>();
    return logger;
}

Logger build_stdout_and_file_logger(const std::string &report_file_path_string) {
    auto masterLogger = std::make_shared<xpansion::logger::Master>();
    Logger loggerFile = std::make_shared<xpansion::logger::UserFile>(report_file_path_string);
    Logger loggerUser = std::make_shared<xpansion::logger::User>(std::cout);
    masterLogger->addLogger(loggerFile);
    masterLogger->addLogger(loggerUser);
    Logger logger = masterLogger;
    return logger;
}