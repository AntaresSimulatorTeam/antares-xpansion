
#include "BendersFactory.h"

int main(int argc, char **argv) {
  mpi::environment env(argc, argv);
  mpi::communicator world;

  if (world.rank() == 0) {
    int i;
    std::cout << "enter an int\n";
    std::cin >> i;
  }
  world.barrier();
  auto benders_factory =
      BendersMainFactory(argc, argv, BENDERSMETHOD::BENDERSBYBATCH, env, world);

  return benders_factory.Run();
}