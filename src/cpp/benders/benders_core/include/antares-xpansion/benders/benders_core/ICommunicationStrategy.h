#pragma once

#include <string>
#include <vector>

#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"
#include "antares-xpansion/benders/benders_core/common.h"

class ICommunicationStrategy
{
public:
    virtual ~ICommunicationStrategy() = default;

    virtual int Rank() const = 0;
    virtual int WorldSize() const = 0;

    virtual void BroadcastXOut(Point& x_out, int root) const = 0;
    virtual void BroadcastXCut(Point& x_cut, int root) const = 0;
    virtual void BroadcastStop(bool& stop, int root) const = 0;
    virtual void BroadcastDoubleVector(std::vector<double>& vec, int root) const = 0;
    virtual void BroadcastInt(int& value, int root) const = 0;

    virtual void AllReduceSum(double& value) const = 0;
    virtual void AllReduceSum(long long& value) const = 0;

    virtual void Barrier() const = 0;
};
