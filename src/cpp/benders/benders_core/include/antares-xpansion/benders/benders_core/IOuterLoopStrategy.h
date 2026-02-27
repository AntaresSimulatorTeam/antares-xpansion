#pragma once

class IOuterLoopStrategy
{
public:
    virtual ~IOuterLoopStrategy() = default;
    virtual void Run() = 0;
};
