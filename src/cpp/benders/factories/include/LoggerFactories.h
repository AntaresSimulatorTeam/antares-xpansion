
#ifndef ANTARESXPANSION_LOGGERFACTORIES_H
#define ANTARESXPANSION_LOGGERFACTORIES_H

#include "core/ILogger.h"

Logger build_void_logger();

Logger build_stdout_and_file_logger(const std::string &report_file_path_string);
#endif //ANTARESXPANSION_LOGGERFACTORIES_H
