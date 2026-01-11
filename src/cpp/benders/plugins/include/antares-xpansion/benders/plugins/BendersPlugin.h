#pragma once




class BendersPlugin 
{
    public : 
        virtual ~BendersPlugin() = default ;
        virtual void OnBendersStart() = 0 ; 
        virtual void OnBendersEnd() = 0 ;  
        virtual void OnBendersMasterIterationStart() = 0 ;  
        virtual void OnBendersMasterIterationEnd() = 0 ;  
        virtual void OnBendersMicroIterationStart() = 0 ; 
        virtual void OnBendersMicroIterationEnd() = 0 ; 
};