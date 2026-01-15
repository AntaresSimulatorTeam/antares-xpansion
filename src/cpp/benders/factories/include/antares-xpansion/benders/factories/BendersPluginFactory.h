#pragma once


#include <filesystem>
#include <memory>
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include <dlfcn.h>
#include <iostream>


class BendersPluginFactory 
{
    public : 

        BendersPluginFactory( ) ;
        BendersPlugin* CreatePlugin(char** subs_ids, int n_subs) ;

    private :

        std::filesystem::path library_path_ ; 



};

typedef BendersPlugin* (*CreatePluginFunc)() ; 