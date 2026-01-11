#include "antares-xpansion/benders/factories/BendersPluginFactory.h"
#include "antares-xpansion/benders/plugins/BendersPluginTST.h"


BendersPluginFactory::BendersPluginFactory() 
                    
{
    std::cout<<"BendersPluginFactory Constructor"<<std::endl ; 
    library_path_ = "" ; 
}


BendersPlugin* BendersPluginFactory::CreatePlugin() 
{
    BendersPlugin* plugin = new BendersPluginTST() ; 
   #if 0
    void* handle = dlopen(library_path_.c_str(),RTLD_LAZY)  ; 
   
   if (!handle) 
   {
       std::cout<<"cannot find the library file"<<std::endl;  
       dlclose(handle) ; 
       return nullptr ; 
   }

   auto createPlugin = (CreatePluginFunc) dlsym(handle,"CreatePlugin") ; 
   if (!createPlugin) 
   {
        std::cout<<"can't find CreatePlugin in the external library"<<std::endl ; 
        dlclose(handle) ; 
        return nullptr ; 
   }

    BendersPlugin* plugin = createPlugin() ; 
    dlclose(handle) ; 
#endif 
   return plugin; 

}

