#pragma once

class IBatchingStrategy
{
public:
    virtual ~IBatchingStrategy() = default;
    virtual void InitializeProblems() = 0;
    virtual void UpdateStoppingCriterion() = 0;
    virtual bool ShouldRelaxationStop() const = 0;
};

