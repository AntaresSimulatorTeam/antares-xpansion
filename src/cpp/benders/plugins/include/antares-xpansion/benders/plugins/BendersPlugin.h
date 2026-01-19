#pragma once

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
#include <map>
#include <string>
#include <memory>



class BendersPlugin 
{
    public : 
        virtual ~BendersPlugin() = default ;
        virtual void OnBendersStart() = 0 ; 
        virtual void OnBendersEnd() = 0 ;  
        virtual void OnBendersMasterIterationStart(std::map<std::string,double>& ) = 0 ;  
        virtual void OnBendersMasterIterationEnd() = 0 ;  
        virtual void OnBendersMicroIterationStart() = 0 ; 
        virtual void OnBendersMicroIterationEnd(std::shared_ptr<ConstraintsReader> constraint_reader, std::string sub_name) = 0 ; 
};