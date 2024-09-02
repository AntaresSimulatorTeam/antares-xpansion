#include "BendersFactory.h"
#include "common_mpi.h"

int main(int argc, char **argv) {
  int i = 0;
  //while (!i)
  //  sleep(5);
  mpi::environment env(argc, argv);
  mpi::communicator world;
  auto benders_factory =
      BendersMainFactory(argc, argv, env, world, SOLVER::BENDERS);
  return benders_factory.Run();
}
