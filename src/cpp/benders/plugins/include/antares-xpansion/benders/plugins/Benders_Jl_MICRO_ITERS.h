#pragma once 
#include "antares-xpansion/benders/plugins/BendersPlugin.h"

class Benders_Jl_MICRO_ITERS : public BendersPlugin 
{
    public : 
        Benders_Jl_MICRO_ITERS() ; 
        virtual ~Benders_Jl_MICRO_ITERS()  ;
        virtual void OnBendersStart()  ; 
        virtual void OnBendersEnd()  ;  
        virtual void OnBendersMasterIterationStart()  ;  
        virtual void OnBendersMasterIterationEnd()  ;  
        virtual void OnBendersMicroIterationStart()  ; 
        virtual void OnBendersMicroIterationEnd()  ; 
} ; 