
#include "antares-xpansion/benders/factories/BendersPluginFactory.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"

BendersPluginFactory::BendersPluginFactory(const SimulationOptions& options) : options_(options)  

{

    input_root_ = options_.INPUTROOT;
    output_root_ = options_.OUTPUTROOT; 
    library_path_ = input_root_ / "libmylib/lib/libmylib.so";
}

BendersPlugin* BendersPluginFactory::CreatePlugin(const CouplingMap& coupling_map, bool micro_iter)
{
    if (micro_iter)
    {
        std::cout << "from createPlugin " << std::endl;
        int n_subs = coupling_map.size();
        const char** subs_ids = (const char**)malloc(n_subs * sizeof(const char*));
        int sub_pos = 0;
        for (auto& [sub_name, sub_variables_map]: coupling_map)
        {
            if (sub_name != "master")
            {
                std::cout << "sub_name " << sub_name << std::endl;
                subs_ids[sub_pos] = sub_name.c_str();
                sub_pos++;
            }
        }

        Benders_Jl_MICRO_ITERS* plugin_jl_micro_iters = new Benders_Jl_MICRO_ITERS(options_,
                                                                                   coupling_map);
        plugin_jl_micro_iters->SetSubProblemIDs(subs_ids, sub_pos);

        BendersPlugin* plugin = (BendersPlugin*)plugin_jl_micro_iters;

        return plugin;
    }

    return nullptr;
}
