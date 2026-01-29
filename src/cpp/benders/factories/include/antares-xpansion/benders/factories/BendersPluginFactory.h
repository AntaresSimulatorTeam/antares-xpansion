#pragma once

#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <memory>

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"


/*
    Factory to create the plugin needed in benders callbacks.
    Called in BendersFactory
*/
class BendersPluginFactory
{
public:
    
    /*
        Constructor
        @inputs : 
            - options : study simulation 
    */
    BendersPluginFactory(const SimulationOptions& options);

    /*
        This method will be called to instantiate the benders plugin 
        @inputs : 
            - coupling_map : coupling map 
            - micro_iter : boolean to check if Micro iterations is needed to build the right plugin
    */
    std::shared_ptr<BendersPlugin> CreatePlugin(const CouplingMap& coupling_map, bool micro_iter, boost::mpi::communicator* world);

private:
    const SimulationOptions& options_ ; 
};

typedef BendersPlugin* (*CreatePluginFunc)();
