#pragma once


#include <filesystem>
#include <memory>
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include <dlfcn.h>
#include <iostream>


class BendersPluginFactory 
{
    public : 

        BendersPluginFactory(const std::filesystem::path& input_root) ;
        BendersPlugin* CreatePlugin(const CouplingMap& coupling_map,bool micro_iter) ;

    private :

        std::filesystem::path library_path_ ; 
        std::filesystem::path input_root_ ; 


};

typedef BendersPlugin* (*CreatePluginFunc)() ; 