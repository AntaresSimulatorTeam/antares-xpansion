
#include "antares-xpansion/benders/factories/BendersPluginFactory.h"
#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"
#include <filesystem>
#include "antares-xpansion/benders/benders_core/common.h"


BendersPluginFactory::BendersPluginFactory() 

{
    std::cout<<"BendersPluginFactory Constructor"<<std::endl ; 
    library_path_ = "" ; 
}


BendersPlugin* BendersPluginFactory::CreatePlugin(char** subs_ids, int n_subs) 
{

    std::filesystem::path jl_lib_path = "./libmylib/lib/libmylib.so"; 
    Benders_Jl_MICRO_ITERS* plugin_jl_microè_iters = new Benders_Jl_MICRO_ITERS(jl_lib_path) ; 
    plugin_jl_microè_iters->SetSubProblemIDs(subs_ids,n_subs) ; 
    
    BendersPlugin* plugin = (BendersPlugin*) plugin_jl_microè_iters ; 

   return  plugin; 

}
