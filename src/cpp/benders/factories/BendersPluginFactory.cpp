
#include "antares-xpansion/benders/factories/BendersPluginFactory.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"

BendersPluginFactory::BendersPluginFactory(const SimulationOptions& options) : options_(options)  

{
}

std::shared_ptr<BendersPlugin> BendersPluginFactory::CreatePlugin(const CouplingMap& coupling_map, bool micro_iter)
{
    if (micro_iter)
    {
        std::cout << "from createPlugin " << std::endl;
        int n_subs = coupling_map.size();
        std::vector<const char*> subs_ids ; 
        subs_ids.reserve(n_subs) ; 
        for (auto& [sub_name, sub_variables_map]: coupling_map)
        {
            if (sub_name != "master")
            {
                std::cout << "sub_name " << sub_name << std::endl;
                subs_ids.push_back(sub_name.c_str());
            }
        }

        std::shared_ptr<Benders_Jl_MICRO_ITERS>  plugin_jl_micro_iters = std::make_shared<Benders_Jl_MICRO_ITERS> (options_,
                                                                                   coupling_map);
        plugin_jl_micro_iters->SetSubProblemIDs(subs_ids.data(), subs_ids.size());

        std::shared_ptr<BendersPlugin> plugin = plugin_jl_micro_iters;

        return plugin;
    }

    return nullptr;
}
