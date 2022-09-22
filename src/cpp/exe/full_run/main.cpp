#include <exception>
#include <iostream>
#include <string>

#include "BendersMpiMain.h"
#include "BendersSequentialMain.h"
#include "ProblemGenerationMain.h"

int main(int argc, char** argv) {
  auto ret = ProblemGenerationMain(argc, argv);
  if (ret != 0) {
    std::string msg =
        "Error Problem Generation returned: " + std::to_string(ret);
    throw std::runtime_error(msg);
  }
  return 0;
}
