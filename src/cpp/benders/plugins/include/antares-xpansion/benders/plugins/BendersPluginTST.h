#pragma once 

#include "antares-xpansion/benders/plugins/BendersPlugin.h"

class BendersPluginTST : public BendersPlugin
{
    public : 
        BendersPluginTST() ; 
        virtual ~BendersPluginTST() ;
        virtual void OnBendersStart()  ; 
        virtual void OnBendersEnd()  ;  
        virtual void OnBendersMasterIterationStart()  ;  
        virtual void OnBendersMasterIterationEnd()  ;  
        virtual void OnBendersMicroIterationStart()  ; 
        virtual void OnBendersMicroIterationEnd()  ; 
};
