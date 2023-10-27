
#include <iostream>

#include "ProblemGeneration.h"
#include "ProblemGenerationExeOptions.h"

int main(int argc, char** argv) {
  try {
    auto options_parser = ProblemGenerationExeOptions();
    options_parser.Parse(argc, argv);

    ProblemGeneration pbg(options_parser);
    pbg.updateProblems();

    return 0;
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}