
#include "antares-xpansion/benders/factories/BendersPluginFactory.h"
#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"
#include <filesystem>
#include "antares-xpansion/benders/benders_core/common.h"


BendersPluginFactory::BendersPluginFactory(const std::filesystem::path& input_root) 

{
    library_path_ = input_root / "libmylib/lib/libmylib.so" ; 
    input_root_ = input_root ; 
}


BendersPlugin* BendersPluginFactory::CreatePlugin(const char** subs_ids, int n_subs) 
{

    Benders_Jl_MICRO_ITERS* plugin_jl_micro_iters = new Benders_Jl_MICRO_ITERS(input_root_) ; 
    plugin_jl_micro_iters->SetSubProblemIDs(subs_ids,n_subs) ; 
    
    BendersPlugin* plugin = (BendersPlugin*) plugin_jl_micro_iters ; 

   return  plugin; 

}
