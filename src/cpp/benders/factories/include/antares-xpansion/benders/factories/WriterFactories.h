
#ifndef ANTARESXPANSION_WRITERFACTORIES_H
#define ANTARESXPANSION_WRITERFACTORIES_H

#include <filesystem>
#include <string>

#include "antares-xpansion/benders/output/OutputWriter.h"

std::shared_ptr<Output::OutputWriter> build_void_writer();

std::shared_ptr<Output::OutputWriter> build_json_writer(const std::filesystem::path& json_file_name,
                                                        bool restart);

#endif // ANTARESXPANSION_WRITERFACTORIES_H
