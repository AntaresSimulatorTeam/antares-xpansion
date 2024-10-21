
#ifndef ANTARESXPANSION_WRITERFACTORIES_H
#define ANTARESXPANSION_WRITERFACTORIES_H

#include <filesystem>
#include <string>

#include "antares-xpansion/xpansion_interfaces/OutputWriter.h"

Writer build_void_writer();

Writer build_json_writer(const std::filesystem::path &json_file_name,
                         const bool restart);

#endif  // ANTARESXPANSION_WRITERFACTORIES_H
