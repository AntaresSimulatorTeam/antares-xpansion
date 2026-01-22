#pragma once

#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <memory>

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"

class BendersPluginFactory
{
public:
    BendersPluginFactory(const std::filesystem::path& input_root, const std::filesystem::path& output_root);
    BendersPlugin* CreatePlugin(const CouplingMap& coupling_map, bool micro_iter);

private:
    std::filesystem::path library_path_;
    std::filesystem::path input_root_;
    std::filesystem::path output_root_;
};

typedef BendersPlugin* (*CreatePluginFunc)();
