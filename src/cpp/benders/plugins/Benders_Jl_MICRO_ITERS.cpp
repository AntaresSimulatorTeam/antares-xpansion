#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"
#include "iostream"


Benders_Jl_MICRO_ITERS::Benders_Jl_MICRO_ITERS() 
{

}

Benders_Jl_MICRO_ITERS::~Benders_Jl_MICRO_ITERS()
{

}

void Benders_Jl_MICRO_ITERS::OnBendersStart() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersStart"<<std::endl ;  
}

void Benders_Jl_MICRO_ITERS::OnBendersEnd()
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersEnd"<<std::endl ; 
}


void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationStart() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERSOnBendersMasterIterationStart"<<std::endl ;  
}

void Benders_Jl_MICRO_ITERS::OnBendersMasterIterationEnd() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMasterIterationEnd"<<std::endl ; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationStart() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMicroIterationStart"<<std::endl; 
}

void Benders_Jl_MICRO_ITERS::OnBendersMicroIterationEnd() 
{
    std::cout<<"from Benders_Jl_MICRO_ITERS OnBendersMicroIterationEnd"<<std::endl ; 
}


