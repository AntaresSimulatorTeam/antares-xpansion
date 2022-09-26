
#include <iostream>

#include "ProblemGenerationExeOptions.h"
#include "RunProblemGeneration.h"

int main(int argc, char** argv) {
  try {
    auto options_parser = ProblemGenerationExeOptions();
    options_parser.Parse(argc, argv);

    auto root = options_parser.Root();
    auto master_formulation = options_parser.MasterFormulation();
    auto additionalConstraintFilename_l =
        options_parser.additional_constraintFilename_l();
    RunProblemGeneration(root, master_formulation,
                         additionalConstraintFilename_l);

    return 0;
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}