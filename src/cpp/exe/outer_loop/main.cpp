#include "BendersFactory.h"
#include "common_mpi.h"

int main(int argc, char **argv) {
  mpi::environment env(argc, argv);
  mpi::communicator world;
  auto benders_factory =
      BendersMainFactory(argc, argv, env, world, SOLVER::ADEQUACY_CRITERION);
  return benders_factory.Run();
}
