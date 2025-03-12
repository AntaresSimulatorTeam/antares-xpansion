#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/factories/BendersFactory.h"

int main(int argc, char** argv)
{
    mpi::environment env(argc, argv);
    mpi::communicator world;
    // First check usage (options are given)
    if (world.rank() == 0)
    {
        usage(argc);
    }
    auto benders_factory = BendersMainFactory(argv, env, world, SOLVER::BENDERS);
    return benders_factory.Run();
}
