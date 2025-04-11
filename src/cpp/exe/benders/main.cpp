#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/factories/BendersFactory.h"

int main(int argc, char** argv)
{
    mpi::environment env(argc, argv);
    mpi::communicator world;
    auto benders_factory = BendersMainFactory(argc, argv, world, SOLVER::BENDERS);
    return benders_factory.Run();
}
