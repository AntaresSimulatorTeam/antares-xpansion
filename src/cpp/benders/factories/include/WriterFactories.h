
#ifndef ANTARESXPANSION_WRITERFACTORIES_H
#define ANTARESXPANSION_WRITERFACTORIES_H

#include <filesystem>
#include <string>

#include "OutputWriter.h"

Writer build_void_writer();

Writer build_json_writer(const std::filesystem::path &json_file_name);

#endif  // ANTARESXPANSION_WRITERFACTORIES_H
