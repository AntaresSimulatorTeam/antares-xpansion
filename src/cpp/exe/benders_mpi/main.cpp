#include "BendersMpiMain.h"
#include "common_mpi.h"

int main(int argc, char **argv) {
  mpi::environment env(argc, argv);
  mpi::communicator world;
  return BendersMpiMain(argc, argv, argv[1], env, world);
}