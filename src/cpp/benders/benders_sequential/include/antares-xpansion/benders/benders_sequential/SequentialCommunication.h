#pragma once

#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"

class SequentialCommunication: public ICommunicationStrategy
{
public:
    int Rank() const override
    {
        return 0;
    }

    int WorldSize() const override
    {
        return 1;
    }

    void BroadcastXOut(Point& x_out, int root) const override
    { /* Nothing to do */
    }

    void BroadcastXCut(Point& x_cut, int root) const override
    { /* Nothing to do */
    }

    void BroadcastStop(bool& stop, int root) const override
    { /* Nothing to do */
    }

    void BroadcastDoubleVector(std::vector<double>& vec, int root) const override
    { /* Nothing to do */
    }

    void BroadcastInt(int& value, int root) const override
    { /* Nothing to do */
    }

    void AllReduceSum(double& value) const override
    { /* Nothing to do */
    }

    void AllReduceSum(long long& value) const override
    { /* Nothing to do */
    }

    void Barrier() const override
    { /* Nothing to do */
    }
};
