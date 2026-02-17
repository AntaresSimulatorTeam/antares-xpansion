#pragma once

#include "BendersStructsDatas.h"

namespace Output
{
class IOutputWriter
{
public:
    virtual ~IOutputWriter() = default;
    virtual void writeIteration(const Output::Iteration& it) = 0;
    virtual void flush() = 0;
};
} // namespace Output

