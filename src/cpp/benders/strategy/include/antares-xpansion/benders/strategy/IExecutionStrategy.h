#pragma once

#include <string>

class IExecutionStrategy
{
public:
    virtual ~IExecutionStrategy() = default;
    virtual void launch() = 0;
    virtual void InitializeProblems() = 0;
    virtual void Run() = 0;
    virtual std::string BendersName() const = 0;
    virtual double execution_time() const = 0;
};

