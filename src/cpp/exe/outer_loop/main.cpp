#include <exception>
#include <iostream>

#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/factories/BendersApp.h"

int main(int argc, char** argv)
{
    try
    {
        mpi::environment env(argc, argv);
        mpi::communicator world;
        // First check usage (options are given)
        if (world.rank() == 0)
        {
            usage(argc);
        }
        auto benders_factory = BendersApp(argv[1], world, SOLVER::OUTER_LOOP);
        return benders_factory.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
        return 1;
    }
}
