#pragma once


#include <filesystem>
#include <memory>
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include <dlfcn.h>
#include <iostream>


class BendersPluginFactory 
{
    public : 

        BendersPluginFactory(const std::filesystem::path& input_root) ;
        BendersPlugin* CreatePlugin(const char** subs_ids, int n_subs) ;

    private :

        std::filesystem::path library_path_ ; 
        std::filesystem::path input_root_ ; 


};

typedef BendersPlugin* (*CreatePluginFunc)() ; 