#pragma once

#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"

class MpiCommunication: public ICommunicationStrategy
{
public:
    explicit MpiCommunication(mpi::communicator& world):
        _world(world)
    {
    }

    int Rank() const override
    {
        return _world.rank();
    }

    int WorldSize() const override
    {
        return _world.size();
    }

    void BroadcastXOut(Point& x_out, int root) const override
    {
        mpi::broadcast(_world, x_out, root);
    }

    void BroadcastXCut(Point& x_cut, int root) const override
    {
        mpi::broadcast(_world, x_cut, root);
    }

    void BroadcastStop(bool& stop, int root) const override
    {
        mpi::broadcast(_world, stop, root);
    }

    void BroadcastDoubleVector(std::vector<double>& vec, int root) const override
    {
        mpi::broadcast(_world, vec, root);
    }

    void BroadcastInt(int& value, int root) const override
    {
        mpi::broadcast(_world, value, root);
    }

    void AllReduceSum(double& value) const override
    {
        double out;
        mpi::all_reduce(_world, value, out, std::plus<double>());
        value = out;
    }

    void AllReduceSum(long long& value) const override
    {
        long long out;
        mpi::all_reduce(_world, value, out, std::plus<long long>());
        value = out;
    }

    void Barrier() const override
    {
        _world.barrier();
    }

private:
    mpi::communicator& _world;
};
