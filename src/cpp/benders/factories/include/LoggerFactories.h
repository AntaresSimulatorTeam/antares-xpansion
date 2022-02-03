
#ifndef ANTARESXPANSION_LOGGERFACTORIES_H
#define ANTARESXPANSION_LOGGERFACTORIES_H

#include "BendersOptions.h"
#include "core/ILogger.h"

Logger build_void_logger();

Logger build_stdout_and_file_logger(const std::string &report_file_path_string);
std::ostringstream start_message(const BendersOptions &options,
                                 const std::string &benders_type);
#endif  // ANTARESXPANSION_LOGGERFACTORIES_H
