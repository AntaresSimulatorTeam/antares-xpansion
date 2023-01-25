#include "BendersFactory.h"

int main(int argc, char **argv) {
  auto benders_factory =
      BendersMainFactory(argc, argv, BENDERSMETHOD::SEQUENTIAL);
  return benders_factory.Run();
}