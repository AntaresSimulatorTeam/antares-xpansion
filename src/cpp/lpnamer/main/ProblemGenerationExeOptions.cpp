#include "ProblemGenerationExeOptions.h"
namespace po = boost::program_options;

ProblemGenerationExeOptions::ProblemGenerationExeOptions()
    : OptionsParser("Problem Generation exe") {
  AddOptions()("help,h", "produce help message")(
      "output,o", po::value<std::filesystem::path>(&root_)->required(),
      "antares-xpansion study output")(
      "formulation,f",
      po::value<std::string>(&master_formulation_)->default_value("relaxed"),
      "master formulation (relaxed or integer)")(
      "exclusion-files,e",
      po::value<std::string>(&additional_constraintFilename_l_),
      "path to exclusion files");
}