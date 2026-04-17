
#include "antares-xpansion/benders/factories/BendersPluginFactory.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/plugins/Benders_MICRO_ITERS.h"
#include "antares-xpansion/benders/plugins/NoOperationPlugin.h"

BendersPluginFactory::BendersPluginFactory(const SimulationOptions& options):
    options_(options)
{
}

std::shared_ptr<BendersPlugin> BendersPluginFactory::CreatePlugin(const CouplingMap& coupling_map,
                                                                  bool micro_iter,
                                                                  boost::mpi::communicator* world)
{
    std::cout << "creating plugin ..... " << std::endl;
    if (micro_iter)
    {
        int n_subs = coupling_map.size();
        std::vector<const char*> subs_ids;
        subs_ids.reserve(n_subs);
        for (auto& [sub_name, sub_variables_map]: coupling_map)
        {
            if (sub_name != "master")
            {
                subs_ids.push_back(sub_name.c_str());
            }
        }

        std::shared_ptr<Benders_MICRO_ITERS>
          plugin_micro_iters = std::make_shared<Benders_MICRO_ITERS>(options_,
                                                                           coupling_map,
                                                                           world);
        plugin_micro_iters->SetSubProblemIDs(subs_ids.data(), subs_ids.size());

        std::shared_ptr<BendersPlugin> plugin = plugin_micro_iters;

        return plugin;
    }

    std::shared_ptr<BendersPlugin> noOp_plugin = std::make_shared<NoOperationPlugin>();
    return noOp_plugin;
}
