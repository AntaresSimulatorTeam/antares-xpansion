#pragma once

class IOuterLoopStrategy
{
public:
    virtual ~IOuterLoopStrategy() = default;
    virtual void Run() = 0;
    virtual void RunAttachedAlgo() = 0;
    virtual bool UpdateMaster() = 0;
    virtual void PrintLog() = 0;
    virtual void init_data() = 0;
    virtual bool isExceptionRaised() = 0;
    virtual double OuterLoopLambdaMin() const = 0;
    virtual double OuterLoopLambdaMax() const = 0;
    virtual void OuterLoopCheckFeasibility() = 0;
    virtual void OuterLoopBilevelChecks() = 0;
};
