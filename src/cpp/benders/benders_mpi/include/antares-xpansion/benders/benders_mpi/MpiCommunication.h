#pragma once

#include <memory>
#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"

class MpiCommunication: public ICommunicationStrategy
{
public:
    // Construct from reference (backward compatible) by copying into a shared_ptr
    explicit MpiCommunication(mpi::communicator& world):
        _world_ptr(std::make_shared<mpi::communicator>(world))
    {
    }

    // Construct from shared_ptr (preferred) to share ownership of the communicator
    explicit MpiCommunication(std::shared_ptr<mpi::communicator> world_ptr):
        _world_ptr(std::move(world_ptr))
    {
    }

    int Rank() const override
    {
        return _world_ptr->rank();
    }

    int WorldSize() const override
    {
        return _world_ptr->size();
    }

    void BroadcastXOut(Point& x_out, int root) const override
    {
        mpi::broadcast(*_world_ptr, x_out, root);
    }

    void BroadcastXCut(Point& x_cut, int root) const override
    {
        mpi::broadcast(*_world_ptr, x_cut, root);
    }

    void BroadcastStop(bool& stop, int root) const override
    {
        mpi::broadcast(*_world_ptr, stop, root);
    }

    void BroadcastDoubleVector(std::vector<double>& vec, int root) const override
    {
        mpi::broadcast(*_world_ptr, vec, root);
    }

    void BroadcastInt(int& value, int root) const override
    {
        mpi::broadcast(*_world_ptr, value, root);
    }

    void AllReduceSum(double& value) const override
    {
        double out;
        mpi::all_reduce(*_world_ptr, value, out, std::plus<>());
        value = out;
    }

    void AllReduceSum(long long& value) const override
    {
        long long out;
        mpi::all_reduce(*_world_ptr, value, out, std::plus<>());
        value = out;
    }

    void Barrier() const override
    {
        _world_ptr->barrier();
    }

private:
    std::shared_ptr<mpi::communicator> _world_ptr;
};
