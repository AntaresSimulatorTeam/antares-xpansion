
#include "antares-xpansion/benders/factories/BendersPluginFactory.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_core/common.h"

BendersPluginFactory::BendersPluginFactory(const SimulationOptions& options):
    options_(options)

{
}

std::shared_ptr<BendersPlugin> BendersPluginFactory::CreatePlugin(const CouplingMap& coupling_map,
                                                                  bool micro_iter,
                                                                  boost::mpi::communicator* world)
{
    return nullptr;
}
