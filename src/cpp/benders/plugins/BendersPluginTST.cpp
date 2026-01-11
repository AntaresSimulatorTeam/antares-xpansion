#include "antares-xpansion/benders/plugins/BendersPluginTST.h"
#include <iostream>


BendersPluginTST::BendersPluginTST()
{
    std::cout<<"building BendersPluginTST object "<<std::endl ; 
}

BendersPluginTST::~BendersPluginTST()
{
    std::cout<<"destroying BendersPluginTST object"<<std::endl ;
}

void BendersPluginTST::OnBendersStart() 
{
    std::cout<<"OnBendersStart from BendersPluginTST"<<std::endl ; 
}

void BendersPluginTST::OnBendersEnd() 
{
    std::cout<<"OnBendersEnd from BendersPluginTST"<<std::endl ; 
}

void BendersPluginTST::OnBendersMasterIterationStart() 
{
    std::cout<<"OnBendersMasterIterationStart from BendersPluginTST"<<std::endl; 
}

void BendersPluginTST::OnBendersMasterIterationEnd() 
{
    std::cout<<"OnBendersMasterIterationEnd from BendersPluginTST "<<std::endl; 
}

void BendersPluginTST::OnBendersMicroIterationStart() 
{
    std::cout<<"OnBendersMicroIterationStart from BendersPluginTST "<<std::endl ; 
}

void BendersPluginTST::OnBendersMicroIterationEnd() 
{
    std::cout<<"OnBendersMicroIterationEnd from BendersPluginTST "<<std::endl; 
}